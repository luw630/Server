#pragma    once
#include "Producer.h"
#include "Consumer.h"


class CPeer : public CProducer,CConsumer
{
private:

public:
    void Run();
    BOOL Initialize();
    void UnInitialize();
};