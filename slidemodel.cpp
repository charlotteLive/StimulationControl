#include "slidemodel.h"

Slidemodel::Slidemodel(const double la, const int k):
    lama(la),
    ks(k),
    b(0.02/0.26)
{
    error = 0;derror=0;
    s = derror + lama*error;

    bj = 15;
    gama = 1;

    //rbf网络参数初始化
    for(int i = 0;i<5;i++){
        w[i] = 1;
        cij[i] = -2 + 1*i;
        h[i] = 1;
    }

}

int Slidemodel::computeOut(double desire1, double desire2, double desire3, double theta, double dtheta)
{
    error = desire1 - theta;
    derror = desire2 - dtheta;
    s = 0.3*derror + lama*error;
    //f = -[mgl*sin(theta) + k(theta - omiga) + B*dtheta]/j
    double f = -(7.42*sin(theta) + 2.47*(theta - 0.5) + 0.19*dtheta)/0.26;    //吴强的数据
//    double f = -(7.56*sin(theta) + 2.72*(theta - 0.3) + 0.27*dtheta)/0.265;    //吴林辉的数据
//    double f = -(8.05*sin(theta) + 3.54*(theta - 0.22) + 0.26*dtheta)/0.293;    //刘润丰的数据
    fn = rbf();
//    uvss = ks*sat(s);
//    uvss = 2*sign(s) + 10*s;
    uvss = 2*qPow(qAbs(s),0.5)*sign(s) + ks*s;
    int u = (desire3 - f -fn + lama*derror + uvss)/b + 50;
    updatarbf();
    if(u<50)    u=50;
    if(u>450)   u=450;
    return u;
}

double Slidemodel::sat(double ss)
{

    double delta = 0.3;
    if(fabs(ss)>=delta)    return sign(ss);
    else return 1/delta*ss;
}

int Slidemodel::sign(double ss)
{
    if(ss>0)    return 1;
    else if(ss=0)   return 0;
    else    return -1;
}

double Slidemodel::rbf()
{
    double fnn = 0;
    for(int i = 0;i<5;i++)
    {
        h[i] = qSqrt(qPow(error - cij[i],2) + qPow(error - cij[i],2));
        h[i] = -qPow(h[i],2)/(2*bj*bj);
        h[i] = qExp(h[i]);
        fnn += w[i]*h[i];
    }
    return fnn;
}

void Slidemodel::updatarbf()
{
    for(int i = 0;i<5;i++){
        w[i] += -1/gama*s*h[i]*0.04;
    }
}

