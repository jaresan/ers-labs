#ifndef PIDController_H_
#define PIDController_H_

class PIDController {
public:
    PIDController(double, double, double);

    void setOutputRange(double, double);

    void reset();

    double getOutput(double, double);

private:
    double kP, kI, kD;
    double minOut, maxOut;
    double lastValue, errorSum;
};

#endif
