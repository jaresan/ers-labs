//
// Created by Antonín Jareš on 2019-06-01.
//

#include "PIDController.h"

PIDController::PIDController(double p, double i, double d): kP(p), kI(i), kD(d) {
    errorSum = 0;
    lastValue = 0;
}

void PIDController::setOutputRange(double min, double max) {
    maxOut = max;
    minOut = min;
}

double PIDController::getOutput(double currentValue, double target) {
    double output;
    double P, I, D;

    double error = target - currentValue;

    P = kP * error;
    I = kI * errorSum;
    D = -kD * (currentValue - lastValue);

    output = P + I + D;

    // Reset the error if the output is out of bounds
    if (output < minOut || output > maxOut) {
        errorSum = error;
    } else {
        errorSum += error;
    }

    if (output > maxOut) {
        output = maxOut;
    } else if (output < minOut) {
        output = minOut;
    }

    lastValue = currentValue;

    return output;
}

void PIDController::reset() {
    errorSum = 0;
    lastValue = 0;
}