# **README**

Refer to this file for online reinforcement learning development, training in runtime, and testing in runtime.

## **Quick Start**

> How to train reinforcement learning model online/run testing code in runtime?

Open a terminal in this folder and start python code by the following code

```bash
bash> cd ./src
bash> python3 train.py  			# run this command for online RL training
bash> python3 test.py <model name>	# run this command for testing in runtime
```

Open another terminal in OpenROAD-flow-script and run the following code

```bash
bash> pwd
.../OpenROAD-flow-scripts/
bash> build_openroad.sh --local		# run this command for the first time. Omit this for second and further running
bash> make							# change makefile to select design to run
```

## **For Development**

<img src="/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/intermediate/doc/data transfer diagram.jpg" style="zoom:30%;" />

This is how C++ and python code run and how they communicate with each other through intermediate files. 

![](/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/dqn/intermediate/doc/code base state machine.jpg)

This is the state machine for C++/Python/bash code to run.

You have to finish the following tasks to finish development

- [ ] finish a helloWorld version of the code set
- [ ] modify C++ code in OpenROAD
- [ ] modify python code to include data preprocessing code and model training/testing code
- [ ] test training and testing

## **Init File Format**

> {7,  0,  3,      shapeCost,               0,       shapeCost, 0.950, RipUpMode::ALL    ,  true}, //  0
>
> {7, -2,  3,      shapeCost,       shapeCost,       shapeCost, 0.950, RipUpMode::ALL    ,  true}, //  1
>
> {7, -5,  3,      shapeCost,       shapeCost,       shapeCost, 0.950, RipUpMode::ALL    ,  true}, //  2
>
> {7,  0,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  3
>
> {7, -1,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  4
>
> {7, -2,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  5
>
> {7, -3,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  6
>
> {7, -4,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  7
>
> {7, -5,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  8
>
> {7, -6,  8,      shapeCost,      MARKERCOST,   2 * shapeCost, 0.950, RipUpMode::DRC    , false}, //  9
>
> {7,  0,  8,  2 * shapeCost,      MARKERCOST,   3 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 10
>
> {7, -1,  8,  2 * shapeCost,      MARKERCOST,   3 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 11
>
> {7, -2,  8,  2 * shapeCost,      MARKERCOST,   3 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 12
>
> {7, -3,  8,  2 * shapeCost,      MARKERCOST,   3 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 13
>
> {7, -4,  8,  2 * shapeCost,      MARKERCOST,   3 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 14
>
> {7, -5,  8,  2 * shapeCost,      MARKERCOST,   4 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 15
>
> {7, -6,  8,  2 * shapeCost,      MARKERCOST,   4 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 16
>
> {7, -3,  8,      shapeCost,      MARKERCOST,   4 * shapeCost, 0.950, RipUpMode::ALL    , false}, // 17
>
> {7,  0,  8,  4 * shapeCost,      MARKERCOST,   4 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 18
>
> {7, -1,  8,  4 * shapeCost,      MARKERCOST,   4 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 19
>
> {7, -2,  8,  4 * shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 20
>
> {7, -3,  8,  4 * shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 21
>
> {7, -4,  8,  4 * shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 22
>
> {7, -5,  8,      shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::NEARDRC, false}, // 23
>
> {7, -6,  8,  4 * shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 24
>
> {5, -2,  8,      shapeCost,      MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::ALL    , false}, // 25
>
> {7,  0,  8,  8 * shapeCost,  2 * MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 26
>
> {7, -1,  8,  8 * shapeCost,  2 * MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 27
>
> {7, -2,  8,  8 * shapeCost,  2 * MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 28
>
> {7, -3,  8,  8 * shapeCost,  2 * MARKERCOST,  10 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 29
>
> {7, -4,  8,      shapeCost,      MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::NEARDRC, false}, // 30
>
> {7, -5,  8,  8 * shapeCost,  2 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 31
>
> {7, -6,  8,  8 * shapeCost,  2 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 32
>
> {3, -1,  8,      shapeCost,      MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::ALL    , false}, // 33
>
> {7,  0,  8, 16 * shapeCost,  4 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 34
>
> {7, -1,  8, 16 * shapeCost,  4 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 35
>
> {7, -2,  8, 16 * shapeCost,  4 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 36
>
> {7, -3,  8,      shapeCost,      MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::NEARDRC, false}, // 37
>
> {7, -4,  8, 16 * shapeCost,  4 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 38
>
> {7, -5,  8, 16 * shapeCost,  4 * MARKERCOST,  50 * shapeCost, 0.950, RipUpMode::DRC    , false}, // 39
>
> {7, -6,  8, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 40
>
> {3, -2,  8,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::ALL    , false}, // 41
>
> {7,  0, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 42
>
> {7, -1, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 43
>
> {7, -2, 16,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::NEARDRC, false}, // 44
>
> {7, -3, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 45
>
> {7, -4, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 46
>
> {7, -5, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 47
>
> {7, -6, 16, 16 * shapeCost,  4 * MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::DRC    , false}, // 48
>
> {3, -0,  8,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.990, RipUpMode::ALL    , false}, // 49
>
> {7,  0, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 50
>
> {7, -1, 32,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::NEARDRC, false}, // 51
>
> {7, -2, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 52
>
> {7, -3, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 53
>
> {7, -4, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 54
>
> {7, -5, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 55
>
> {7, -6, 32, 32 * shapeCost,  8 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 56
>
> {3, -1,  8,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::ALL    , false}, // 57
>
> {7,  0, 64,      shapeCost,      MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::NEARDRC, false}, // 58
>
> {7, -1, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 59
>
> {7, -2, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 60
>
> {7, -3, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 61
>
> {7, -4, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 62
>
> {7, -5, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}, // 63
>
> {7, -6, 64, 64 * shapeCost, 16 * MARKERCOST, 100 * shapeCost, 0.999, RipUpMode::DRC    , false}  // 64

## **Dump File Format**

The columns of dump file has the following items:

> agentCount action envCount drc 
>
> // they are space separated integers

```tex
4 12 
3 45 0 54213
2 65 2 0
1 25 1 271
0 32 0 14001
```

