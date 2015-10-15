#include "StdAfx.h"
#include "biguanarea.h"

CBiGuanArea::CBiGuanArea(void)
{
}

CBiGuanArea::~CBiGuanArea(void)
{
}

int CBiGuanArea::OnCreate(_W64 long pParameter)
{
    CRegion::OnCreate(pParameter);
    SetExperMul(2);

    return 1;
}
