#ifndef SILDEMODEL_H
#define SILDEMODEL_H
#include <QtMath>

class Slidemodel
{
public:
    Slidemodel(const double la,const int k);
    ~Slidemodel();
    int computeOut(double desire1,double desire2,double desire3,double theta,double dtheta);

private:
    double sat(double ss);
    int sign(double ss);
    double rbf();
    void updatarbf();//更新神经网络权值

private:
    double lama; //滑模面参数
    int ks;


    double b;
    double error;
    double derror;
    double uvss;
    double s;//滑模面 s=e + lama*de
    double fn; //神经网络输出

    double bj;
    double gama;
    double w[5];
    double cij[5];
    double h[5];



};

#endif // SILDEMODEL_H
