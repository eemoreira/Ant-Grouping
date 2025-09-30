#pragma once
#ifdef FOUR

const double ALPHA = 25.8029;
const double K1 = 0.02;
const double K2 = 0.03;
const int RADIUS = 2;
const int BOX_SIZE = (RADIUS + 2) * (RADIUS + 2);
const int N = 64;
const int ANT_NUMBER = 150;
const int NUMBER_OF_STEPS = 2e6;
const int PRINT_EVERY = 1e5;

#elif FIFTEEN

const double ALPHA = 0.999;
const double K1 = 0.09;
const double K2 = 0.08;
const int RADIUS = 1;
const int BOX_SIZE = (RADIUS + 2) * (RADIUS + 2);
const int N = 80;
const int ANT_NUMBER = 120;
const int NUMBER_OF_STEPS = 2e7;
const int PRINT_EVERY = 1e6;

#endif

#define sq(x) ((x)*(x))
