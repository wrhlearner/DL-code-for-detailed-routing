#include <iostream>
#include <vector>
typedef unsigned int frUInt4;

enum class RipUpMode
{
  DRC = 0,
  ALL = 1,
  NEARDRC = 2
};

namespace FlexDR{
    struct SearchRepairArgs{
        int size;
        int offset;
        int mazeEndIter;
        frUInt4 workerDRCCost;
        frUInt4 workerMarkerCost;
        frUInt4 workerFixedShapeCost;
        float workerMarkerDecay;
        RipUpMode ripupMode;
        bool followGuide;
    };
}

int FlexDR::main()
{
  ProfileTask profile("DR:main");
  init();
  frTime t;

  // initialize action settings
  std::vector<FlexDR::SearchRepairArgs> args;


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