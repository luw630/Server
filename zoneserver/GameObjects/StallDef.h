#pragma once

#define MAX_SALEITEMNUM 64
#define MAX_SALEBOXWIDTH 8
#define MAX_SALEBOXHEIGHT 8

struct ITEM_POS {
    WORD wPosX;
    WORD wPosY;
};

struct ITEM_BASEINFO {
    DWORD dwItemID;
    DWORD dwValue;
};

struct SALE_ITEM {
    ITEM_POS stSrcPos;
    ITEM_POS stDestPos;
    ITEM_BASEINFO stBaseInfo;
};

struct SALE_ADDITEM {
    ITEM_POS stSrcPos;
    ITEM_BASEINFO stBaseInfo;
};

struct SALE_BOTHSIDES {
    DWORD dwOtherGlobalID;
    DWORD dwSelfGlobalID;
};

struct SALE_SEEITEM {
    SALE_BOTHSIDES stBothSides;
    SALE_ITEM stItemInfo[MAX_SALEITEMNUM];
};

struct SALE_BUYITEM {
    ITEM_POS stDestPos;
    ITEM_BASEINFO stBaseInfo;   
    SALE_BOTHSIDES stBothSides;
};
