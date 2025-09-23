#pragma once
#ifdef FOUR

const double ALPHA = 35.8029;
const double K1 = 0.05;
const int RADIUS = 1;
const int BOX_SIZE = (RADIUS + 2) * (RADIUS + 2);
const int N = 64;
const int ANT_NUMBER = 150;
const int NUMBER_OF_STEPS = 2e6;
const int PRINT_EVERY = 1e5;
const double K2 = 0.03;

#elif FIFTEEN

const double ALPHA = 2.8029;
const double K1 = 0.05;
const int RADIUS = 1;
const int BOX_SIZE = (RADIUS + 2) * (RADIUS + 2);
const int N = 80;
const int ANT_NUMBER = 120;
const int NUMBER_OF_STEPS = 2e6;
const int PRINT_EVERY = 1e5;
const double K2 = 0.1;

#endif

#define sq(x) ((x)*(x))
