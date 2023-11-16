/* Authors: Lutong Wang and Bangqi Xu */
/*
 * Copyright (c) 2019, The Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dr/FlexDR.h"

#include <dst/JobMessage.h>
#include <omp.h>
#include <stdio.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/io/ios_state.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "db/infra/frTime.h"
#include "distributed/RoutingJobDescription.h"
#include "distributed/frArchive.h"
#include "dr/FlexDR_conn.h"
#include "dr/FlexDR_graphics.h"
#include "dst/BalancerJobDescription.h"
#include "dst/Distributed.h"
#include "frProfileTask.h"
#include "gc/FlexGC.h"
#include "io/io.h"
#include "ord/OpenRoad.hh"
#include "serialization.h"
#include "utl/exception.h"

using namespace std;
using namespace fr;

using utl::ThreadException;

BOOST_CLASS_EXPORT(RoutingJobDescription)

enum class SerializationType
{
  READ,
  WRITE
};

void serializeWorker(FlexDRWorker* worker, std::string& workerStr)
{
  std::stringstream stream(std::ios_base::binary | std::ios_base::in
                           | std::ios_base::out);
  frOArchive ar(stream);
  registerTypes(ar);
  ar << *worker;
  workerStr = stream.str();
}

void deserializeWorker(FlexDRWorker* worker,
                       frDesign* design,
                       const std::string& workerStr)
{
  std::stringstream stream(
      workerStr,
      std::ios_base::binary | std::ios_base::in | std::ios_base::out);
  frIArchive ar(stream);
  ar.setDesign(design);
  registerTypes(ar);
  ar >> *worker;
}

void serializeViaData(FlexDRViaData viaData, std::string& serializedStr)
{
  std::stringstream stream(std::ios_base::binary | std::ios_base::in
                           | std::ios_base::out);
  frOArchive ar(stream);
  registerTypes(ar);
  ar << viaData;
  serializedStr = stream.str();
}

FlexDR::FlexDR(triton_route::TritonRoute* router,
               frDesign* designIn,
               Logger* loggerIn,
               odb::dbDatabase* dbIn)
    : router_(router),
      design_(designIn),
      logger_(loggerIn),
      db_(dbIn),
      numWorkUnits_(0),
      dist_(nullptr),
      dist_on_(false),
      dist_port_(0),
      increaseClipsize_(false),
      clipSizeInc_(0),
      iter_(0)
{
}

FlexDR::~FlexDR()
{
}

void FlexDR::setDebug(frDebugSettings* settings)
{
  bool on = settings->debugDR;
  graphics_
      = on && FlexDRGraphics::guiActive()
            ? std::make_unique<FlexDRGraphics>(settings, design_, db_, logger_)
            : nullptr;
}

std::string FlexDRWorker::reloadedMain()
{
  init(design_);
  debugPrint(logger_,
             utl::DRT,
             "autotuner",
             1,
             "Init number of markers {}",
             getInitNumMarkers());
  if (!skipRouting_) {
    route_queue();
  }
  setGCWorker(nullptr);
  cleanup();
  std::string workerStr;
  serializeWorker(this, workerStr);
  return workerStr;
}

void FlexDRWorker::writeUpdates(const std::string& file_name)
{
  std::vector<std::vector<drUpdate>> updates(1);
  for (const auto& net : getDesign()->getTopBlock()->getNets()) {
    for (const auto& guide : net->getGuides()) {
      for (const auto& connFig : guide->getRoutes()) {
        drUpdate update(drUpdate::ADD_GUIDE);
        frPathSeg* pathSeg = static_cast<frPathSeg*>(connFig.get());
        update.setPathSeg(*pathSeg);
        update.setIndexInOwner(guide->getIndexInOwner());
        update.setNet(net.get());
        design_->addUpdate(update);
      }
    }
    for (const auto& shape : net->getShapes()) {
      drUpdate update;
      auto pathSeg = static_cast<frPathSeg*>(shape.get());
      update.setPathSeg(*pathSeg);
      update.setNet(net.get());
      update.setUpdateType(drUpdate::ADD_SHAPE);
      updates.back().push_back(update);
    }
    for (const auto& shape : net->getVias()) {
      drUpdate update;
      auto via = static_cast<frVia*>(shape.get());
      update.setVia(*via);
      update.setNet(net.get());
      update.setUpdateType(drUpdate::ADD_SHAPE);
      updates.back().push_back(update);
    }
    for (const auto& shape : net->getPatchWires()) {
      drUpdate update;
      auto patch = static_cast<frPatchWire*>(shape.get());
      update.setPatchWire(*patch);
      update.setNet(net.get());
      update.setUpdateType(drUpdate::ADD_SHAPE);
      updates.back().push_back(update);
    }
  }
  for (const auto& marker : getDesign()->getTopBlock()->getMarkers()) {
    drUpdate update;
    update.setMarker(*(marker.get()));
    update.setUpdateType(drUpdate::ADD_SHAPE);
    updates.back().push_back(update);
  }
  std::ofstream file(file_name.c_str());
  frOArchive ar(file);
  registerTypes(ar);
  ar << updates;
  file.close();
}

int FlexDRWorker::main(frDesign* design)
{
  ProfileTask profile("DRW:main");
  using namespace std::chrono;
  high_resolution_clock::time_point t0 = high_resolution_clock::now();
  auto micronPerDBU = 1.0 / getTech()->getDBUPerUU();
  if (VERBOSE > 1) {
    logger_->report("start DR worker (BOX) ( {} {} ) ( {} {} )",
                    routeBox_.xMin() * micronPerDBU,
                    routeBox_.yMin() * micronPerDBU,
                    routeBox_.xMax() * micronPerDBU,
                    routeBox_.yMax() * micronPerDBU);
  }
  initMarkers(design);
  if (getDRIter() && getInitNumMarkers() == 0 && !needRecheck_) {
    skipRouting_ = true;
  }
  if (debugSettings_->debugDumpDR
      && (debugSettings_->x == -1
          || routeBox_.intersects({debugSettings_->x, debugSettings_->y}))
      && !skipRouting_ && debugSettings_->iter == getDRIter()) {
    std::string workerPath = fmt::format("{}/workerx{}_y{}",
                                         debugSettings_->dumpDir,
                                         routeBox_.xMin(),
                                         routeBox_.yMin());
    mkdir(workerPath.c_str(), 0777);

    writeUpdates(fmt::format("{}/updates.bin", workerPath));
    {
      std::string viaDataStr;
      serializeViaData(*via_data_, viaDataStr);
      ofstream viaDataFile(fmt::format("{}/viadata.bin", workerPath).c_str());
      viaDataFile << viaDataStr;
      viaDataFile.close();
    }
    {
      std::string workerStr;
      serializeWorker(this, workerStr);
      ofstream workerFile(fmt::format("{}/worker.bin", workerPath).c_str());
      workerFile << workerStr;
      workerFile.close();
    }
    {
      std::ofstream globalsFile(
          fmt::format("{}/worker_globals.bin", workerPath).c_str());
      frOArchive ar(globalsFile);
      registerTypes(ar);
      serializeGlobals(ar);
      globalsFile.close();
    }
  }
  if (!skipRouting_) {
    init(design);
  }
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  if (!skipRouting_) {
    route_queue();
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  const int num_markers = getNumMarkers();
  cleanup();
  high_resolution_clock::time_point t3 = high_resolution_clock::now();

  duration<double> time_span0 = duration_cast<duration<double>>(t1 - t0);
  duration<double> time_span1 = duration_cast<duration<double>>(t2 - t1);
  duration<double> time_span2 = duration_cast<duration<double>>(t3 - t2);

  if (VERBOSE > 1) {
    stringstream ss;
    ss << "time (INIT/ROUTE/POST) " << time_span0.count() << " "
       << time_span1.count() << " " << time_span2.count() << " " << endl;
    cout << ss.str() << flush;
  }

  debugPrint(logger_,
             DRT,
             "autotuner",
             1,
             "worker ({:.3f} {:.3f}) ({:.3f} {:.3f}) time {} prev_#DRVs {} "
             "curr_#DRVs {}",
             routeBox_.xMin() * micronPerDBU,
             routeBox_.yMin() * micronPerDBU,
             routeBox_.xMax() * micronPerDBU,
             routeBox_.yMax() * micronPerDBU,
             duration_cast<duration<double>>(t3 - t0).count(),
             getInitNumMarkers(),
             num_markers);

  return 0;
}

void FlexDRWorker::distributedMain(frDesign* design)
{
  ProfileTask profile("DR:main");
  if (VERBOSE > 1) {
    logger_->report("start DR worker (BOX) ( {} {} ) ( {} {} )",
                    routeBox_.xMin() * 1.0 / getTech()->getDBUPerUU(),
                    routeBox_.yMin() * 1.0 / getTech()->getDBUPerUU(),
                    routeBox_.xMax() * 1.0 / getTech()->getDBUPerUU(),
                    routeBox_.yMax() * 1.0 / getTech()->getDBUPerUU());
  }
  initMarkers(design);
  if (getDRIter() && getInitNumMarkers() == 0 && !needRecheck_) {
    skipRouting_ = true;
    return;
  }
}

void FlexDR::initFromTA()
{
  // initialize lists
  for (auto& net : getDesign()->getTopBlock()->getNets()) {
    for (auto& guide : net->getGuides()) {
      for (auto& connFig : guide->getRoutes()) {
        if (connFig->typeId() == frcPathSeg) {
          unique_ptr<frShape> ps = make_unique<frPathSeg>(
              *(static_cast<frPathSeg*>(connFig.get())));
          auto [bp, ep] = static_cast<frPathSeg*>(ps.get())->getPoints();

          // skip TA dummy segment
          if (Point::manhattanDistance(ep, bp) != 1) {
            net->addShape(std::move(ps));
          }
        } else {
          cout << "Error: initFromTA unsupported shape" << endl;
        }
      }
    }
  }
}

void FlexDR::initGCell2BoundaryPin()
{
  // initialize size
  auto topBlock = getDesign()->getTopBlock();
  auto gCellPatterns = topBlock->getGCellPatterns();
  auto& xgp = gCellPatterns.at(0);
  auto& ygp = gCellPatterns.at(1);
  auto tmpVec
      = vector<map<frNet*, set<pair<Point, frLayerNum>>, frBlockObjectComp>>(
          (int) ygp.getCount());
  gcell2BoundaryPin_ = vector<
      vector<map<frNet*, set<pair<Point, frLayerNum>>, frBlockObjectComp>>>(
      (int) xgp.getCount(), tmpVec);
  for (auto& net : topBlock->getNets()) {
    auto netPtr = net.get();
    for (auto& guide : net->getGuides()) {
      for (auto& connFig : guide->getRoutes()) {
        if (connFig->typeId() == frcPathSeg) {
          auto ps = static_cast<frPathSeg*>(connFig.get());
          auto [bp, ep] = ps->getPoints();
          frLayerNum layerNum = ps->getLayerNum();
          // skip TA dummy segment
          auto mdist = Point::manhattanDistance(ep, bp);
          if (mdist == 1 || mdist == 0) {
            continue;
          }
          Point idx1 = design_->getTopBlock()->getGCellIdx(bp);
          Point idx2 = design_->getTopBlock()->getGCellIdx(ep);

          // update gcell2BoundaryPin
          // horizontal
          if (bp.y() == ep.y()) {
            int x1 = idx1.x();
            int x2 = idx2.x();
            int y = idx1.y();
            for (auto x = x1; x <= x2; ++x) {
              Rect gcellBox = topBlock->getGCellBox(Point(x, y));
              frCoord leftBound = gcellBox.xMin();
              frCoord rightBound = gcellBox.xMax();
              const bool hasLeftBound = bp.x() < leftBound;
              const bool hasRightBound = ep.x() >= rightBound;
              if (hasLeftBound) {
                Point boundaryPt(leftBound, bp.y());
                gcell2BoundaryPin_[x][y][netPtr].emplace(boundaryPt, layerNum);
              }
              if (hasRightBound) {
                Point boundaryPt(rightBound, ep.y());
                gcell2BoundaryPin_[x][y][netPtr].emplace(boundaryPt, layerNum);
              }
            }
          } else if (bp.x() == ep.x()) {
            int x = idx1.x();
            int y1 = idx1.y();
            int y2 = idx2.y();
            for (auto y = y1; y <= y2; ++y) {
              Rect gcellBox = topBlock->getGCellBox(Point(x, y));
              frCoord bottomBound = gcellBox.yMin();
              frCoord topBound = gcellBox.yMax();
              const bool hasBottomBound = bp.y() < bottomBound;
              const bool hasTopBound = ep.y() >= topBound;
              if (hasBottomBound) {
                Point boundaryPt(bp.x(), bottomBound);
                gcell2BoundaryPin_[x][y][netPtr].emplace(boundaryPt, layerNum);
              }
              if (hasTopBound) {
                Point boundaryPt(ep.x(), topBound);
                gcell2BoundaryPin_[x][y][netPtr].emplace(boundaryPt, layerNum);
              }
            }
          } else {
            cout << "Error: non-orthogonal pathseg in initGCell2BoundryPin\n";
          }
        }
      }
    }
  }
}

void FlexDR::init_halfViaEncArea()
{
  auto bottomLayerNum = getTech()->getBottomLayerNum();
  auto topLayerNum = getTech()->getTopLayerNum();
  auto& halfViaEncArea = via_data_.halfViaEncArea;
  for (int i = bottomLayerNum; i <= topLayerNum; i++) {
    if (getTech()->getLayer(i)->getType() != dbTechLayerType::ROUTING) {
      continue;
    }
    if (i + 1 <= topLayerNum
        && getTech()->getLayer(i + 1)->getType() == dbTechLayerType::CUT) {
      auto viaDef = getTech()->getLayer(i + 1)->getDefaultViaDef();
      frVia via(viaDef);
      Rect layer1Box = via.getLayer1BBox();
      Rect layer2Box = via.getLayer2BBox();
      auto layer1HalfArea = layer1Box.area() / 2;
      auto layer2HalfArea = layer2Box.area() / 2;
      halfViaEncArea.push_back(make_pair(layer1HalfArea, layer2HalfArea));
    } else {
      halfViaEncArea.push_back(make_pair(0, 0));
    }
  }
}

void FlexDR::init()
{
  ProfileTask profile("DR:init");
  frTime t;
  if (VERBOSE > 0) {
    logger_->info(DRT, 187, "Start routing data preparation.");
  }
  initGCell2BoundaryPin();
  getRegionQuery()->initDRObj();  // first init in postProcess

  init_halfViaEncArea();

  if (VERBOSE > 0) {
    t.print(logger_);
  }

  iter_ = 0;

  if (VERBOSE > 0) {
    logger_->info(DRT, 194, "Start detail routing.");
  }
}

void FlexDR::removeGCell2BoundaryPin()
{
  gcell2BoundaryPin_.clear();
  gcell2BoundaryPin_.shrink_to_fit();
}

map<frNet*, set<pair<Point, frLayerNum>>, frBlockObjectComp>
FlexDR::initDR_mergeBoundaryPin(int startX,
                                int startY,
                                int size,
                                const Rect& routeBox)
{
  map<frNet*, set<pair<Point, frLayerNum>>, frBlockObjectComp> bp;
  auto gCellPatterns = getDesign()->getTopBlock()->getGCellPatterns();
  auto& xgp = gCellPatterns.at(0);
  auto& ygp = gCellPatterns.at(1);
  for (int i = startX; i < (int) xgp.getCount() && i < startX + size; i++) {
    for (int j = startY; j < (int) ygp.getCount() && j < startY + size; j++) {
      auto& currBp = gcell2BoundaryPin_[i][j];
      for (auto& [net, s] : currBp) {
        for (auto& [pt, lNum] : s) {
          if (pt.x() == routeBox.xMin() || pt.x() == routeBox.xMax()
              || pt.y() == routeBox.yMin() || pt.y() == routeBox.yMax()) {
            bp[net].emplace(pt, lNum);
          }
        }
      }
    }
  }
  return bp;
}

void FlexDR::getBatchInfo(int& batchStepX, int& batchStepY)
{
  batchStepX = 2;
  batchStepY = 2;
}

void FlexDR::searchRepair(const SearchRepairArgs& args)
{
  const int iter = iter_++;
  const int size = args.size;
  const int offset = args.offset;
  const int mazeEndIter = args.mazeEndIter;
  const frUInt4 workerDRCCost = args.workerDRCCost;
  const frUInt4 workerMarkerCost = args.workerMarkerCost;
  const frUInt4 workerFixedShapeCost = args.workerFixedShapeCost;
  const float workerMarkerDecay = args.workerMarkerDecay;
  const RipUpMode ripupMode = args.ripupMode;
  const bool followGuide = args.followGuide;

  std::string profile_name("DR:searchRepair");
  profile_name += std::to_string(iter);
  ProfileTask profile(profile_name.c_str());
  if (ripupMode != RipUpMode::ALL
      && getDesign()->getTopBlock()->getMarkers().empty()) {
    return;
  }
  if (dist_on_) {
    if ((iter % 10 == 0 && iter != 60) || iter == 3 || iter == 15) {
      globals_path_ = fmt::format("{}globals.{}.ar", dist_dir_, iter);
      router_->writeGlobals(globals_path_.c_str());
    }
  }
  frTime t;
  if (VERBOSE > 0) {
    string suffix;
    if (iter == 1 || (iter > 20 && iter % 10 == 1)) {
      suffix = "st";
    } else if (iter == 2 || (iter > 20 && iter % 10 == 2)) {
      suffix = "nd";
    } else if (iter == 3 || (iter > 20 && iter % 10 == 3)) {
      suffix = "rd";
    } else {
      suffix = "th";
    }
    logger_->info(DRT, 195, "Start {}{} optimization iteration.", iter, suffix);
  }
  if (graphics_) {
    graphics_->startIter(iter);
  }
  auto gCellPatterns = getDesign()->getTopBlock()->getGCellPatterns();
  auto& xgp = gCellPatterns.at(0);
  auto& ygp = gCellPatterns.at(1);
  int cnt = 0;
  int tot = (((int) xgp.getCount() - 1 - offset) / size + 1)
            * (((int) ygp.getCount() - 1 - offset) / size + 1);
  int prev_perc = 0;
  bool isExceed = false;

  vector<unique_ptr<FlexDRWorker>> uworkers;
  int batchStepX, batchStepY;

  getBatchInfo(batchStepX, batchStepY);

  vector<vector<vector<unique_ptr<FlexDRWorker>>>> workers(batchStepX
                                                           * batchStepY);

  int xIdx = 0, yIdx = 0;
  for (int i = offset; i < (int) xgp.getCount(); i += size) {
    for (int j = offset; j < (int) ygp.getCount(); j += size) {
      auto worker = make_unique<FlexDRWorker>(&via_data_, design_, logger_);
      Rect routeBox1 = getDesign()->getTopBlock()->getGCellBox(Point(i, j));
      const int max_i = min((int) xgp.getCount() - 1, i + size - 1);
      const int max_j = min((int) ygp.getCount(), j + size - 1);
      Rect routeBox2
          = getDesign()->getTopBlock()->getGCellBox(Point(max_i, max_j));
      Rect routeBox(routeBox1.xMin(),
                    routeBox1.yMin(),
                    routeBox2.xMax(),
                    routeBox2.yMax());
      Rect extBox;
      Rect drcBox;
      routeBox.bloat(MTSAFEDIST, extBox);
      routeBox.bloat(DRCSAFEDIST, drcBox);
      worker->setRouteBox(routeBox);
      worker->setExtBox(extBox);
      worker->setDrcBox(drcBox);
      worker->setGCellBox(Rect(i, j, max_i, max_j));
      worker->setMazeEndIter(mazeEndIter);
      worker->setDRIter(iter);
      worker->setDebugSettings(router_->getDebugSettings());
      if (dist_on_)
        worker->setDistributed(dist_, dist_ip_, dist_port_, dist_dir_);
      if (!iter) {
        // if (routeBox.xMin() == 441000 && routeBox.yMin() == 816100) {
        //   cout << "@@@ debug: " << i << " " << j << endl;
        // }
        // set boundary pin
        auto bp = initDR_mergeBoundaryPin(i, j, size, routeBox);
        worker->setDRIter(0, bp);
      }
      worker->setRipupMode(ripupMode);
      worker->setFollowGuide(followGuide);
      // TODO: only pass to relevant workers
      worker->setGraphics(graphics_.get());
      worker->setCost(workerDRCCost,
                      workerMarkerCost,
                      workerFixedShapeCost,
                      workerMarkerDecay);

      int batchIdx = (xIdx % batchStepX) * batchStepY + yIdx % batchStepY;
      if (workers[batchIdx].empty()
          || (!dist_on_
              && (int) workers[batchIdx].back().size() >= BATCHSIZE)) {
        workers[batchIdx].push_back(vector<unique_ptr<FlexDRWorker>>());
      }
      workers[batchIdx].back().push_back(std::move(worker));

      yIdx++;
    }
    yIdx = 0;
    xIdx++;
  }

  omp_set_num_threads(MAX_THREADS);
  int version = 0;
  increaseClipsize_ = false;
  numWorkUnits_ = 0;
  // parallel execution
  for (auto& workerBatch : workers) {
    ProfileTask profile("DR:checkerboard");
    for (auto& workersInBatch : workerBatch) {
      {
        const std::string batch_name = std::string("DR:batch<")
                                       + std::to_string(workersInBatch.size())
                                       + ">";
        ProfileTask profile(batch_name.c_str());
        if (dist_on_) {
          router_->dist_pool_.join();
          if (version++ == 0 && !design_->hasUpdates()) {
            std::string serializedViaData;
            serializeViaData(via_data_, serializedViaData);
            router_->sendGlobalsUpdates(globals_path_, serializedViaData);
          } else
            router_->sendDesignUpdates(globals_path_);
        }
        {
          ProfileTask task("DIST: PROCESS_BATCH");
          // multi thread
          ThreadException exception;
#pragma omp parallel for schedule(dynamic)
          for (int i = 0; i < (int) workersInBatch.size(); i++) {
            try {
              if (dist_on_)
                workersInBatch[i]->distributedMain(getDesign());
              else
                workersInBatch[i]->main(getDesign());
#pragma omp critical
              {
                cnt++;
                if (VERBOSE > 0) {
                  if (cnt * 1.0 / tot >= prev_perc / 100.0 + 0.1
                      && prev_perc < 90) {
                    if (prev_perc == 0 && t.isExceed(0)) {
                      isExceed = true;
                    }
                    prev_perc += 10;
                    if (isExceed) {
                      logger_->report(
                          "    Completing {}% with {} violations.",
                          prev_perc,
                          getDesign()->getTopBlock()->getNumMarkers());
                      logger_->report("    {}.", t);
                    }
                  }
                }
              }
            } catch (...) {
              exception.capture();
            }
          }
          exception.rethrow();
          if (dist_on_) {
            int j = 0;
            std::vector<std::vector<std::pair<int, FlexDRWorker*>>>
                distWorkerBatches(router_->getCloudSize());
            for (int i = 0; i < workersInBatch.size(); i++) {
              auto worker = workersInBatch.at(i).get();
              if (!worker->isSkipRouting()) {
                distWorkerBatches[j].push_back({i, worker});
                j = (j + 1) % router_->getCloudSize();
              }
            }
            {
              ProfileTask task("DIST: SERIALIZE+SEND");
#pragma omp parallel for schedule(dynamic)
              for (int i = 0; i < distWorkerBatches.size(); i++)
                sendWorkers(distWorkerBatches.at(i), workersInBatch);
            }
            logger_->report("    Received Batches:{}.", t);
            std::vector<std::pair<int, std::string>> workers;
            router_->getWorkerResults(workers);
            {
              ProfileTask task("DIST: DESERIALIZING_BATCH");
#pragma omp parallel for schedule(dynamic)
              for (int i = 0; i < workers.size(); i++) {
                deserializeWorker(workersInBatch.at(workers.at(i).first).get(),
                                  design_,
                                  workers.at(i).second);
              }
            }
            logger_->report("    Deserialized Batches:{}.", t);
          }
        }
      }
      {
        ProfileTask profile("DR:end_batch");
        // single thread
        for (int i = 0; i < (int) workersInBatch.size(); i++) {
          if (workersInBatch[i]->end(getDesign()))
            numWorkUnits_ += 1;
          if (workersInBatch[i]->isCongested())
            increaseClipsize_ = true;
        }
        workersInBatch.clear();
      }
    }
  }

  if (!iter) {
    removeGCell2BoundaryPin();
  }
  if (VERBOSE > 0) {
    if (cnt * 1.0 / tot >= prev_perc / 100.0 + 0.1 && prev_perc >= 90) {
      if (prev_perc == 0 && t.isExceed(0)) {
        isExceed = true;
      }
      prev_perc += 10;
      if (isExceed) {
        logger_->report("    Completing {}% with {} violations.",
                        prev_perc,
                        getDesign()->getTopBlock()->getNumMarkers());
        logger_->report("    {}.", t);
      }
    }
  }
  FlexDRConnectivityChecker checker(
      getDesign(),
      logger_,
      db_,
      graphics_.get(),
      dist_on_ || router_->getDebugSettings()->debugDumpDR);
  checker.check(iter);
  numViols_.push_back(getDesign()->getTopBlock()->getNumMarkers());
  debugPrint(logger_,
             utl::DRT,
             "workers",
             1,
             "Number of work units = {}.",
             numWorkUnits_);
  if (VERBOSE > 0) {
    logger_->info(DRT,
                  199,
                  "  Number of violations = {}.",
                  getDesign()->getTopBlock()->getNumMarkers());
    if (getDesign()->getTopBlock()->getNumMarkers() > 0) {
      // report violations
      std::map<std::string, std::map<frLayerNum, uint>> violations;
      std::set<frLayerNum> layers;
      const std::map<std::string, std::string> relabel
          = {{"Lef58SpacingEndOfLine", "EOL"},
             {"Lef58CutSpacingTable", "CutSpcTbl"},
             {"Lef58EolKeepOut", "eolKeepOut"}};
      for (const auto& marker : getDesign()->getTopBlock()->getMarkers()) {
        if (!marker->getConstraint())
          continue;
        auto type = marker->getConstraint()->getViolName();
        if (relabel.find(type) != relabel.end())
          type = relabel.at(type);
        violations[type][marker->getLayerNum()]++;
        layers.insert(marker->getLayerNum());
      }
      std::string line = fmt::format("{:<15}", "Viol/Layer");
      for (auto lNum : layers) {
        std::string lName = getTech()->getLayer(lNum)->getName();
        if (lName.size() >= 7) {
          lName = lName.substr(0, 2) + ".." + lName.substr(lName.size() - 2, 2);
        }
        line += fmt::format("{:>7}", lName);
      }
      logger_->report(line);
      for (auto [type, typeViolations] : violations) {
        std::string typeName = type;
        if (typeName.size() >= 15)
          typeName = typeName.substr(0, 12) + "..";
        line = fmt::format("{:<15}", typeName);
        for (auto lNum : layers) {
          line += fmt::format("{:>7}", typeViolations[lNum]);
        }
        logger_->report(line);
      }
    }
    t.print(logger_);
    cout << flush;
  }
  end();
  if ((DRC_RPT_ITER_STEP && iter > 0 && iter % DRC_RPT_ITER_STEP.value() == 0)
      || logger_->debugCheck(DRT, "autotuner", 1)
      || logger_->debugCheck(DRT, "report", 1)) {
    router_->reportDRC(DRC_RPT_FILE + '-' + std::to_string(iter) + ".rpt",
                       design_->getTopBlock()->getMarkers());
  }
}

void FlexDR::end(bool done)
{
  if (done && DRC_RPT_FILE != string("")) {
    router_->reportDRC(DRC_RPT_FILE, design_->getTopBlock()->getMarkers());
  }
  if (done && VERBOSE > 0) {
    logger_->info(DRT, 198, "Complete detail routing.");
  }

  using ULL = unsigned long long;
  const auto size = getTech()->getLayers().size();
  vector<ULL> wlen(size, 0);
  vector<ULL> sCut(size, 0);
  vector<ULL> mCut(size, 0);
  auto topBlock = getDesign()->getTopBlock();
  for (auto& net : topBlock->getNets()) {
    for (auto& shape : net->getShapes()) {
      if (shape->typeId() == frcPathSeg) {
        auto obj = static_cast<frPathSeg*>(shape.get());
        auto [bp, ep] = obj->getPoints();
        auto lNum = obj->getLayerNum();
        frCoord psLen = Point::manhattanDistance(ep, bp);
        wlen[lNum] += psLen;
      }
    }
    for (auto& via : net->getVias()) {
      auto lNum = via->getViaDef()->getCutLayerNum();
      if (via->getViaDef()->isMultiCut()) {
        ++mCut[lNum];
      } else {
        ++sCut[lNum];
      }
    }
  }

  const ULL totWlen = std::accumulate(wlen.begin(), wlen.end(), ULL(0));
  const ULL totSCut = std::accumulate(sCut.begin(), sCut.end(), ULL(0));
  const ULL totMCut = std::accumulate(mCut.begin(), mCut.end(), ULL(0));

  if (done) {
    logger_->metric("route__drc_errors", topBlock->getNumMarkers());
    logger_->metric("route__wirelength", totWlen / topBlock->getDBUPerUU());
    logger_->metric("route__vias", totSCut + totMCut);
    logger_->metric("route__vias__singlecut", totSCut);
    logger_->metric("route__vias__multicut", totMCut);
  } else {
    logger_->metric(fmt::format("route__drc_errors__iter:{}", iter_),
                    topBlock->getNumMarkers());
    logger_->metric(fmt::format("route__wirelength__iter:{}", iter_),
                    totWlen / topBlock->getDBUPerUU());
  }

  if (VERBOSE > 0) {
    logger_->report("Total wire length = {} um.",
                    totWlen / topBlock->getDBUPerUU());

    for (int i = getTech()->getBottomLayerNum();
         i <= getTech()->getTopLayerNum();
         i++) {
      if (getTech()->getLayer(i)->getType() == dbTechLayerType::ROUTING) {
        logger_->report("Total wire length on LAYER {} = {} um.",
                        getTech()->getLayer(i)->getName(),
                        wlen[i] / topBlock->getDBUPerUU());
      }
    }
    logger_->report("Total number of vias = {}.", totSCut + totMCut);
    if (totMCut > 0) {
      logger_->report("Total number of multi-cut vias = {} ({:5.1f}%).",
                      totMCut,
                      totMCut * 100.0 / (totSCut + totMCut));
      logger_->report("Total number of single-cut vias = {} ({:5.1f}%).",
                      totSCut,
                      totSCut * 100.0 / (totSCut + totMCut));
    }
    logger_->report("Up-via summary (total {}):.", totSCut + totMCut);
    int nameLen = 0;
    for (int i = getTech()->getBottomLayerNum();
         i <= getTech()->getTopLayerNum();
         i++) {
      if (getTech()->getLayer(i)->getType() == dbTechLayerType::CUT) {
        nameLen
            = max(nameLen, (int) getTech()->getLayer(i - 1)->getName().size());
      }
    }
    int maxL = 1 + nameLen + 4 + (int) to_string(totSCut).length();
    if (totMCut) {
      maxL += 9 + 4 + (int) to_string(totMCut).length() + 9 + 4
              + (int) to_string(totSCut + totMCut).length();
    }
    std::ostringstream msg;
    if (totMCut) {
      msg << " " << setw(nameLen + 4 + (int) to_string(totSCut).length() + 9)
          << "single-cut";
      msg << setw(4 + (int) to_string(totMCut).length() + 9) << "multi-cut"
          << setw(4 + (int) to_string(totSCut + totMCut).length()) << "total";
    }
    msg << endl;
    for (int i = 0; i < maxL; i++) {
      msg << "-";
    }
    msg << endl;
    for (int i = getTech()->getBottomLayerNum();
         i <= getTech()->getTopLayerNum();
         i++) {
      if (getTech()->getLayer(i)->getType() == dbTechLayerType::CUT) {
        msg << " " << setw(nameLen) << getTech()->getLayer(i - 1)->getName()
            << "    " << setw((int) to_string(totSCut).length()) << sCut[i];
        if (totMCut) {
          msg << " (" << setw(5)
              << (double) ((sCut[i] + mCut[i])
                               ? sCut[i] * 100.0 / (sCut[i] + mCut[i])
                               : 0.0)
              << "%)";
          msg << "    " << setw((int) to_string(totMCut).length()) << mCut[i]
              << " (" << setw(5)
              << (double) ((sCut[i] + mCut[i])
                               ? mCut[i] * 100.0 / (sCut[i] + mCut[i])
                               : 0.0)
              << "%)"
              << "    " << setw((int) to_string(totSCut + totMCut).length())
              << sCut[i] + mCut[i];
        }
        msg << endl;
      }
    }
    for (int i = 0; i < maxL; i++) {
      msg << "-";
    }
    msg << endl;
    msg << " " << setw(nameLen) << ""
        << "    " << setw((int) to_string(totSCut).length()) << totSCut;
    if (totMCut) {
      msg << " (" << setw(5)
          << (double) ((totSCut + totMCut)
                           ? totSCut * 100.0 / (totSCut + totMCut)
                           : 0.0)
          << "%)";
      msg << "    " << setw((int) to_string(totMCut).length()) << totMCut
          << " (" << setw(5)
          << (double) ((totSCut + totMCut)
                           ? totMCut * 100.0 / (totSCut + totMCut)
                           : 0.0)
          << "%)"
          << "    " << setw((int) to_string(totSCut + totMCut).length())
          << totSCut + totMCut;
    }
    msg << endl << endl;
    logger_->report("{}", msg.str());
  }
}

static std::vector<FlexDR::SearchRepairArgs> strategy()
{
  const frUInt4 shapeCost = ROUTESHAPECOST;
    return {
			{7,	0,	51,	14,	41,	0.585586,	RipUpMode::ALL,	true},
			{7,	-1,	2,	10,	11,	0.627127,	RipUpMode::ALL,	true},
			{7,	-2,	32,	13,	23,	0.539039,	RipUpMode::ALL,	true},
			{7,	-3,	12,	4,	42,	0.640140,	RipUpMode::DRC,	false},
			{7,	-4,	57,	8,	10,	0.698198,	RipUpMode::DRC,	false},
			{7,	-5,	30,	4,	28,	0.845846,	RipUpMode::DRC,	false},
			{7,	-6,	23,	6,	17,	0.808308,	RipUpMode::DRC,	false},
			{7,	0,	26,	4,	13,	0.978979,	RipUpMode::DRC,	false},
			{7,	-1,	7,	4,	36,	0.592092,	RipUpMode::DRC,	false},
			{7,	-2,	37,	9,	42,	0.808809,	RipUpMode::DRC,	false},
			{7,	-3,	56,	9,	59,	0.760761,	RipUpMode::DRC,	false},
			{7,	-4,	47,	3,	29,	0.691191,	RipUpMode::DRC,	false},
			{7,	-5,	44,	2,	52,	0.791792,	RipUpMode::DRC,	false},
			{7,	-6,	33,	6,	11,	0.545045,	RipUpMode::DRC,	false},
			{7,	0,	25,	13,	98,	0.955956,	RipUpMode::DRC,	false},
			{7,	-1,	36,	14,	7,	0.961461,	RipUpMode::DRC,	false},
			{7,	-2,	41,	10,	33,	0.733233,	RipUpMode::DRC,	false},
			{7,	-3,	58,	14,	25,	0.710210,	RipUpMode::DRC,	false},
			{7,	-4,	51,	11,	26,	0.892893,	RipUpMode::DRC,	false},
			{7,	-5,	19,	6,	4,	0.983483,	RipUpMode::DRC,	false},
			{7,	-6,	18,	2,	19,	0.783283,	RipUpMode::DRC,	false},
			{7,	0,	31,	4,	62,	0.620621,	RipUpMode::DRC,	false},
			{7,	-1,	58,	4,	70,	0.544044,	RipUpMode::DRC,	false},
			{7,	-2,	30,	13,	21,	0.515015,	RipUpMode::DRC,	false},
			{7,	-3,	16,	11,	11,	0.913413,	RipUpMode::DRC,	false},
			{7,	-4,	21,	11,	23,	0.865365,	RipUpMode::DRC,	false},
			{7,	-5,	18,	4,	10,	0.551552,	RipUpMode::DRC,	false},
			{7,	-6,	6,	7,	42,	0.510010,	RipUpMode::DRC,	false},
			{7,	0,	2,	7,	29,	0.713714,	RipUpMode::DRC,	false},
			{7,	-1,	41,	6,	25,	0.867868,	RipUpMode::DRC,	false},
			{7,	-2,	61,	14,	90,	0.784785,	RipUpMode::DRC,	false},
			{7,	-3,	46,	13,	63,	0.628128,	RipUpMode::DRC,	false},
			{7,	-4,	2,	9,	79,	0.949950,	RipUpMode::DRC,	false},
			{7,	-5,	48,	8,	67,	0.609610,	RipUpMode::DRC,	false},
			{7,	-6,	61,	14,	75,	0.658158,	RipUpMode::DRC,	false},
			{7,	0,	29,	1,	39,	0.895395,	RipUpMode::DRC,	false},
			{7,	-1,	58,	4,	54,	0.915415,	RipUpMode::DRC,	false},
			{7,	-2,	52,	13,	26,	0.949449,	RipUpMode::DRC,	false},
			{7,	-3,	30,	7,	83,	0.669670,	RipUpMode::DRC,	false},
			{7,	-4,	18,	13,	44,	0.576076,	RipUpMode::DRC,	false},
			{7,	-5,	11,	13,	1,	0.504004,	RipUpMode::DRC,	false},
			{7,	-6,	19,	15,	5,	0.665666,	RipUpMode::DRC,	false},
			{7,	0,	36,	8,	7,	0.599099,	RipUpMode::DRC,	false},
			{7,	-1,	30,	12,	21,	0.612112,	RipUpMode::DRC,	false},
			{7,	-2,	3,	6,	76,	0.938438,	RipUpMode::DRC,	false},
			{7,	-3,	21,	2,	98,	0.881381,	RipUpMode::DRC,	false},
			{7,	-4,	56,	5,	27,	0.896897,	RipUpMode::DRC,	false},
			{7,	-5,	34,	3,	34,	0.905906,	RipUpMode::DRC,	false},
			{7,	-6,	31,	13,	43,	0.823323,	RipUpMode::DRC,	false},
			{7,	0,	49,	6,	39,	0.710210,	RipUpMode::DRC,	false},
			{7,	-1,	26,	7,	61,	0.656657,	RipUpMode::DRC,	false},
			{7,	-2,	39,	4,	32,	0.595095,	RipUpMode::DRC,	false},
			{7,	-3,	58,	9,	64,	0.655656,	RipUpMode::DRC,	false},
			{7,	-4,	44,	15,	80,	0.771271,	RipUpMode::DRC,	false},
			{7,	-5,	45,	10,	66,	0.593594,	RipUpMode::DRC,	false},
			{7,	-6,	19,	9,	89,	0.583083,	RipUpMode::DRC,	false},
			{7,	0,	4,	12,	51,	0.707708,	RipUpMode::DRC,	false},
			{7,	-1,	63,	3,	88,	0.754254,	RipUpMode::DRC,	false},
			{7,	-2,	2,	12,	63,	0.903904,	RipUpMode::DRC,	false},
			{7,	-3,	46,	12,	40,	0.870370,	RipUpMode::DRC,	false},
			{7,	-4,	50,	4,	73,	0.531532,	RipUpMode::DRC,	false},
			{7,	-5,	50,	13,	68,	0.958458,	RipUpMode::DRC,	false},
			{7,	-6,	21,	13,	1,	0.981481,	RipUpMode::DRC,	false},
			{7,	0,	39,	15,	72,	0.928929,	RipUpMode::DRC,	false}
  };
}

void addRectToPolySet(gtl::polygon_90_set_data<frCoord>& polySet, Rect rect)
{
  using namespace boost::polygon::operators;
  gtl::polygon_90_data<frCoord> poly;
  vector<gtl::point_data<frCoord>> points;
  for (const auto& point : rect.getPoints()) {
    points.push_back({point.x(), point.y()});
  }
  poly.set(points.begin(), points.end());
  polySet += poly;
}

void FlexDR::reportGuideCoverage()
{
  using namespace boost::polygon::operators;

  const auto numLayers = getTech()->getLayers().size();
  std::vector<unsigned long long> totalAreaByLayerNum(numLayers, 0);
  std::vector<unsigned long long> totalCoveredAreaByLayerNum(numLayers, 0);
  map<frNet*, std::vector<float>> netsCoverage;
  const auto& nets = getDesign()->getTopBlock()->getNets();
  omp_set_num_threads(MAX_THREADS);
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < nets.size(); i++) {
    const auto& net = nets.at(i);
    std::vector<gtl::polygon_90_set_data<frCoord>> routeSetByLayerNum(
        numLayers),
        guideSetByLayerNum(numLayers);
    for (const auto& shape : net->getShapes()) {
      odb::Rect rect = shape->getBBox();
      addRectToPolySet(routeSetByLayerNum[shape->getLayerNum()], rect);
    }
    for (const auto& pwire : net->getPatchWires()) {
      odb::Rect rect = pwire->getBBox();
      addRectToPolySet(routeSetByLayerNum[pwire->getLayerNum()], rect);
    }
    for (const auto& via : net->getVias()) {
      {
        odb::Rect rect = via->getLayer1BBox();
        addRectToPolySet(routeSetByLayerNum[via->getViaDef()->getLayer1Num()],
                         rect);
      }
      {
        odb::Rect rect = via->getLayer2BBox();
        addRectToPolySet(routeSetByLayerNum[via->getViaDef()->getLayer2Num()],
                         rect);
      }
    }

    for (const auto& shape : net->getOrigGuides()) {
      odb::Rect rect = shape.getBBox();
      addRectToPolySet(guideSetByLayerNum[shape.getLayerNum()], rect);
    }

    for (frLayerNum lNum = 0; lNum < numLayers; lNum++) {
      if (getTech()->getLayer(lNum)->getType() != dbTechLayerType::ROUTING
          || lNum > TOP_ROUTING_LAYER)
        continue;
      float coveredPercentage = -1.0;
      unsigned long long routingArea = 0;
      unsigned long long coveredArea = 0;
      if (!routeSetByLayerNum[lNum].empty()) {
        routingArea = gtl::area(routeSetByLayerNum[lNum]);
        coveredArea
            = gtl::area(routeSetByLayerNum[lNum] & guideSetByLayerNum[lNum]);
        if (routingArea == 0.0)
          coveredPercentage = -1.0;
        else
          coveredPercentage = (coveredArea / (double) routingArea) * 100;
      }

#pragma omp critical
      {
        netsCoverage[net.get()].push_back(coveredPercentage);
        totalAreaByLayerNum[lNum] += routingArea;
        totalCoveredAreaByLayerNum[lNum] += coveredArea;
      }
    }
  }

  ofstream file(GUIDE_REPORT_FILE);
  file << "Net,";
  for (const auto& layer : getTech()->getLayers()) {
    if (layer->getType() == dbTechLayerType::ROUTING
        && layer->getLayerNum() <= TOP_ROUTING_LAYER) {
      file << layer->getName() << ",";
    }
  }
  file << std::endl;
  for (auto [net, coverage] : netsCoverage) {
    file << net->getName() << ",";
    for (auto coveredPercentage : coverage)
      if (coveredPercentage < 0.0)
        file << "NA,";
      else
        file << fmt::format("{:.2f}%,", coveredPercentage);
    file << std::endl;
  }
  file << "Total,";
  unsigned long long totalArea = 0;
  unsigned long long totalCoveredArea = 0;
  for (const auto& layer : getTech()->getLayers()) {
    if (layer->getType() == dbTechLayerType::ROUTING
        && layer->getLayerNum() <= TOP_ROUTING_LAYER) {
      if (totalAreaByLayerNum[layer->getLayerNum()] == 0) {
        file << "NA,";
        continue;
      }
      totalArea += totalAreaByLayerNum[layer->getLayerNum()];
      totalCoveredArea += totalCoveredAreaByLayerNum[layer->getLayerNum()];
      auto coveredPercentage
          = (totalCoveredAreaByLayerNum[layer->getLayerNum()]
             / (double) totalAreaByLayerNum[layer->getLayerNum()])
            * 100;
      file << fmt::format("{:.2f}%,", coveredPercentage);
    }
  }
  if (totalArea == 0)
    file << "NA";
  else {
    auto totalCoveredPercentage = (totalCoveredArea / (double) totalArea) * 100;
    file << fmt::format("{:.2f}%,", totalCoveredPercentage);
  }
  file.close();
}

int FlexDR::main()
{
  ProfileTask profile("DR:main");
  init();
  frTime t;

  for (auto& args : strategy()) {
    int clipSize = args.size;
    if (args.ripupMode != RipUpMode::ALL) {
      if (increaseClipsize_) {
        clipSizeInc_ += 2;
      } else
        clipSizeInc_ = max((float) 0, clipSizeInc_ - 0.2f);
      clipSize += min(MAX_CLIPSIZE_INCREASE, (int) round(clipSizeInc_));
    }
    args.size = clipSize;

    searchRepair(args);
    if (getDesign()->getTopBlock()->getNumMarkers() == 0) {
      break;
    }
    if (iter_ > END_ITERATION) {
      break;
    }
    if (logger_->debugCheck(DRT, "snapshot", 1)) {
      io::Writer writer(getDesign(), logger_);
      writer.updateDb(db_, true);
      // insert the stack of vias for bterms above max layer again.
      // all routing is deleted in updateDb, so it is necessary to insert the
      // stack again.
      router_->processBTermsAboveTopLayer(true);
      ord::OpenRoad::openRoad()->writeDb(
          fmt::format("drt_iter{}.odb", iter_ - 1).c_str());
    }
  }

  end(/* done */ true);
  if (!GUIDE_REPORT_FILE.empty())
    reportGuideCoverage();
  if (VERBOSE > 0) {
    t.print(logger_);
    cout << endl;
  }
  return 0;
}

void FlexDR::sendWorkers(
    const std::vector<std::pair<int, FlexDRWorker*>>& remote_batch,
    std::vector<std::unique_ptr<FlexDRWorker>>& batch)
{
  if (remote_batch.empty())
    return;
  std::vector<std::pair<int, std::string>> workers;
  {
    ProfileTask task("DIST: SERIALIZE_BATCH");
    for (auto& [idx, worker] : remote_batch) {
      std::string workerStr;
      serializeWorker(worker, workerStr);
      workers.push_back({idx, workerStr});
    }
  }
  std::string remote_ip = dist_ip_;
  unsigned short remote_port = dist_port_;
  if (router_->getCloudSize() > 1) {
    dst::JobMessage msg(dst::JobMessage::BALANCER),
        result(dst::JobMessage::NONE);
    bool ok = dist_->sendJob(msg, dist_ip_.c_str(), dist_port_, result);
    if (!ok) {
      logger_->error(utl::DRT, 7461, "Balancer failed");
    } else {
      dst::BalancerJobDescription* desc
          = static_cast<dst::BalancerJobDescription*>(
              result.getJobDescription());
      remote_ip = desc->getWorkerIP();
      remote_port = desc->getWorkerPort();
    }
  }
  {
    dst::JobMessage msg(dst::JobMessage::ROUTING),
        result(dst::JobMessage::NONE);
    std::unique_ptr<dst::JobDescription> desc
        = std::make_unique<RoutingJobDescription>();
    RoutingJobDescription* rjd
        = static_cast<RoutingJobDescription*>(desc.get());
    rjd->setWorkers(workers);
    rjd->setSharedDir(dist_dir_);
    rjd->setSendEvery(20);
    msg.setJobDescription(std::move(desc));
    ProfileTask task("DIST: SENDJOB");
    bool ok = dist_->sendJobMultiResult(
        msg, remote_ip.c_str(), remote_port, result);
    if (!ok) {
      logger_->error(utl::DRT, 500, "Sending worker {} failed");
    }
    for (const auto& one_desc : result.getAllJobDescriptions()) {
      RoutingJobDescription* result_desc
          = static_cast<RoutingJobDescription*>(one_desc.get());
      router_->addWorkerResults(result_desc->getWorkers());
    }
  }
}

template <class Archive>
void FlexDRWorker::serialize(Archive& ar, const unsigned int version)
{
  // // We always serialize before calling main on the work unit so various
  // // fields are empty and don't need to be serialized.  I skip these to
  // // save having to write lots of serializers that will never be called.
  // if (!apSVia_.empty() || !nets_.empty() || !owner2nets_.empty()
  //     || !rq_.isEmpty() || gcWorker_) {
  //   logger_->error(DRT, 999, "Can't serialize used worker");
  // }

  // The logger_, graphics_ and debugSettings_ are handled by the caller to
  // use the current ones.
  if (is_loading(ar)) {
    frDesign* design = ar.getDesign();
    design_ = design;
  }
  (ar) & routeBox_;
  (ar) & extBox_;
  (ar) & drcBox_;
  (ar) & gcellBox_;
  (ar) & drIter_;
  (ar) & mazeEndIter_;
  (ar) & followGuide_;
  (ar) & needRecheck_;
  (ar) & skipRouting_;
  (ar) & ripupMode_;
  (ar) & workerDRCCost_;
  (ar) & workerMarkerCost_;
  (ar) & workerFixedShapeCost_;
  (ar) & workerMarkerDecay_;
  (ar) & initNumMarkers_;
  (ar) & nets_;
  (ar) & markers_;
  (ar) & bestMarkers_;
  (ar) & isCongested_;
  if (is_loading(ar)) {
    gridGraph_.setTech(design_->getTech());
    gridGraph_.setWorker(this);
    // boundaryPin_
    int sz = 0;
    (ar) & sz;
    while (sz--) {
      frBlockObject* obj;
      serializeBlockObject(ar, obj);
      std::set<std::pair<Point, frLayerNum>> val;
      (ar) & val;
      boundaryPin_[(frNet*) obj] = val;
    }
    // owner2nets_
    for (auto& net : nets_) {
      owner2nets_[net->getFrNet()].push_back(net.get());
    }
    dist_on_ = true;
  } else {
    // boundaryPin_
    int sz = boundaryPin_.size();
    (ar) & sz;
    for (auto& [net, value] : boundaryPin_) {
      frBlockObject* obj = (frBlockObject*) net;
      serializeBlockObject(ar, obj);
      (ar) & value;
    }
  }
}

std::unique_ptr<FlexDRWorker> FlexDRWorker::load(const std::string& workerStr,
                                                 utl::Logger* logger,
                                                 frDesign* design,
                                                 FlexDRGraphics* graphics)
{
  auto worker = std::make_unique<FlexDRWorker>();
  deserializeWorker(worker.get(), design, workerStr);

  // We need to fix up the fields we want from the current run rather
  // than the stored ones.
  worker->setLogger(logger);
  worker->setGraphics(graphics);

  return worker;
}

// Explicit instantiations
template void FlexDRWorker::serialize<frIArchive>(
    frIArchive& ar,
    const unsigned int file_version);

template void FlexDRWorker::serialize<frOArchive>(
    frOArchive& ar,
    const unsigned int file_version);
