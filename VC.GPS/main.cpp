#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include <string>
#include "injector/injector.hpp"
#include <math.h>
#include <stdint.h>


constexpr int MAX_POINTS = 500;

constexpr unsigned int rwRENDERSTATETEXTURERASTER = 1;
constexpr unsigned int rwPRIMTYPETRIFAN = 5;
constexpr unsigned int D3DRS_ALPHAFUNC = 25;
constexpr unsigned int D3DCMP_GREATER = 5;
constexpr float LINE_WIDTH = 1400.0f;




#define IS_PED_IN_CAR(ped) (*(unsigned char *)((unsigned int)ped + 0x3AC))
#define GET_PED_CAR(ped) (*(CVehicle **)((unsigned int)ped + 0x3A8))

#define BYTEn(x, n)   (*((BYTE*)&(x)+n))






struct RwV2d
{
	float x;
	float y;
};

struct RwV3d
{
	float x;
	float y;
	float z;
};

struct RwD3D9Vertex
{
	float        x;
	float        y;
	float        z;
	float        rhw;
	unsigned int emissiveColor;
	float        u;
	float        v;
};

struct RwMatrix
{
	RwV3d right;
	unsigned int flags;
	RwV3d top;
	char _pad0[4];
	RwV3d at;
	char _pad1[4];
	RwV3d pos;
	char _pad2[4];
};

class CMatrix
{
public:
	RwMatrix       m_sMatrix;
	class CMatrix *m_pMatrix;
	unsigned char  m_bHaveRwMatrix;
	char _pad[3];
};

class CPlaceable
{
public:
	void *vmt;
	CMatrix m_sCoords;
};

typedef CPlaceable CVehicle;
typedef CPlaceable CPed;
typedef CPlaceable CObject;
typedef RwV3d CVector;
typedef RwV2d CVector2D;

enum class ePathNodeType
{
	PATHNODE_VEHICLE_PATH = 0,
	PATHNODE_PED_PATH = 1
};

struct CVector2DShort
{
	signed short x;
	signed short y;
};

struct CVectorShort
{
	signed short x;
	signed short y;
	signed short z;
};

struct CPathNode //size 0x14
{
	short unk1;
	short unk2;
	union {
		CVector2DShort     m_v2dPoint;
		CVectorShort       m_vPoint;
	};
	char unk3;
	char unk4;
	short nextPointIndex;
	char unk5;
	char unk6;
	char flags;
	char unk7;
	char unk8;
	char unk9;
};


enum class eBlipType
{
	BLIP_COORD = 0,
	BLIP_CAR,
	BLIP_PED,
	BLIP_OBJECT
};

struct RadarBlip
{
	unsigned int   m_dwBlipColour;
	unsigned int   m_dwBlipType;
	unsigned int   m_dwEntityHandle;
	CVector		   m_vecInitPos;
	CVector        m_vecCurPos;
	unsigned short m_wIndex;
	unsigned char  m_bBlipBrightness;
	unsigned char  m_bActive;
	unsigned int   m_unk2;
	float          m_f3dMarkerAnimState;
	unsigned short m_wBlipScale;
	unsigned short m_wBlipDisplay;
	unsigned short m_wBlipSprite;
	char _pad[2];
};

struct PathLineInfo
{
	union {
		CVector *targetPoint;
		CVector2D *targetPoint2d;
	};
	unsigned int color;
};

CPathNode *gapPathNodes[MAX_POINTS];
short gwPathNodesCount;
CVector gBlipBestPos;

CVehicle *(__cdecl *FindPlayerVehicle)();
bool (*IsPlayerOnAMission)();
void(__thiscall *DoPathSearch)(void *, unsigned char, CVector, int, CVector, CPathNode **, short *, short, CVehicle *, float *, float, int);
float(__cdecl *RwIm2DGetNearScreenZ)();
void(__cdecl *RwRenderStateSet)(unsigned int, unsigned int);
int(__cdecl *RwD3D9SetRenderState)(unsigned int, unsigned int);
void(__cdecl *SetSpriteVertices)(float, float, float, float, float, float, float, float, unsigned int *, unsigned int *, unsigned int *, unsigned int *);
void(__cdecl *RwIm2DRenderPrimitive)(unsigned int, RwD3D9Vertex *, unsigned int);
CVehicle *(__thiscall *VehicleGetAt)(void *, unsigned int);
CPed *(__thiscall *PedGetAt)(void *, unsigned int);
CObject *(__thiscall *ObjectGetAt)(void *, unsigned int);
unsigned int(__cdecl *GetRadarTraceColour)(unsigned int, unsigned int);
void(__cdecl *DrawRadarMap)();
void(__cdecl *DrawRadarMask)();
void(__cdecl *InitialiseRadar)();
void DrawLine(CVector2D const&, CVector2D const&, float, unsigned int);
void DrawPathLineMask();
void RwIm2DSetNearScreenZ(float);
void ProcessPathfind();
float GetSquaredDistanceBetweenPoints(CVector const&, CVector const&);
bool IsLineInsideRadar(CVector2D const&, CVector2D const&);
PathLineInfo *GetPlaceInfo(PathLineInfo *);
void *gPathfind;
float *gRadarMapZShift;
RwD3D9Vertex *gSpriteVertices;
int *gScreenWidth;
int *gScreenHeight;
void **gVehiclePool;
void **gPedPool;
void **gObjectPool;
unsigned int *gRwEngine;
float *gRadarRange;
RadarBlip *gRadarBlips;

void PlayFrontEndSound(unsigned short frontend, unsigned int volume = 0)
{
	void* audioManager = (void*)0xA10B8A;
	void(__thiscall *pPlayFrontEndSound)(void*, unsigned short, unsigned int) = (void(__thiscall *)(void*, unsigned short, unsigned int))0x5F9960;
	pPlayFrontEndSound(audioManager, frontend, volume);
}

void(__cdecl *AsciiToUnicode)       (const char *ascii, short *pUni);
void(__cdecl *PrintString)          (float x, float y, short *text);
void(__cdecl *SetFontStyle)         (int style);
void(__cdecl *SetScale)             (float w, float h);
void(__cdecl *SetColor)             (unsigned int *color);
void(__cdecl *SetDropShadowPosition)(int position);
void(__cdecl *SetPropOn)            ();

void(*pfDrawInMenu)(float x, float y, short *text);


void TransformRadarPointToScreenSpace(CVector2D & a1, CVector2D const& a2)
{
	if (*(BYTE*)0x869665)
	{
		a1.x = (*(float*)0x68FD0C * *(float*)0x869670 * a2.x + *(float*)0x68FD10 * *(float*)0x869670 + *(float*)0x869674)
			* (float)*gScreenWidth
			* **(float**)0x4C1B1A;
		a1.y = (*(float*)0x869678 - *(float*)0x68FD1C * *(float*)0x869670 - *(float*)0x68FD18 * *(float*)0x869670 * a2.y)
			* (float)*gScreenHeight
			* **(float**)0x4C1B46;
	}
	else
	{
		float v1 = (float)*gScreenWidth * **(float**)0x4C1B1A * *(float*)0x68FD24;
		a1.x = a2.x * v1 * *(float*)0x68FD28 + v1 * *(float*)0x68FD28 + *(float*)0x68FD2C;
		a1.y = *(float*)0x68FD30 * (float)*gScreenHeight * **(float**)0x4C1B46 * *(float*)0x68FD28
			+ (float)*gScreenHeight
			- *(float*)0x68FD34 * (float)*gScreenHeight * **(float**)0x4C1B46
			- a2.y * *(float*)0x68FD30 * (float)*gScreenHeight * **(float**)0x4C1B46 * *(float*)0x68FD28;
	}
}

void TransformRealWorldPointToRadarSpace(CVector2D & a1, CVector2D const& a2)
{
	float v9 = *(float *)0x68FD40 / *(float *)0x974BEC;
	float v10 = (a2.x - *(float *)0x704734) * v9;
	float v11 = (a2.y - *(float *)0x704738) * v9;
	a1.x = *(float *)0x70483C * v11 + *(float *)0x704840 * v10;
	a1.y = *(float *)0x704840 * v11 - *(float *)0x70483C * v10;
}


CVector * GetCamPos()
{
	return (CVector *)(0x7E4688 + 0x7D8);
}

void RwIm2DSetNearScreenZ(float z)
{
	*(float *)(*gRwEngine + 0x18) = z;
}

float GetSquaredDistanceBetweenPoints(CVector const&a, CVector const&b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;
	return dz * dz + dy * dy + dx * dx;
}

bool IsLineInsideRadar(CVector2D const&a, CVector2D const&b)
{
	return a.x * a.x + a.y * a.y <= 0.8f || b.x * b.x + b.y * b.y <= 1.0f;
}

void DrawPathLineMask()
{
	DrawRadarMask();
	unsigned int color;
	CVector2D points[2];
	{
		color = 0;
		points[0].x = 1;
		points[0].y = 1;
		points[1].x = -1;
		points[1].y = -1;
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
		TransformRadarPointToScreenSpace(points[0], points[0]);
		TransformRadarPointToScreenSpace(points[1], points[1]);
		SetSpriteVertices(points[1].x, (float)*gScreenHeight, points[1].x, 0.0f, 0.0f, (float)*gScreenHeight, 0.0f, 0.0f, &color,
			&color, &color, &color);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
		SetSpriteVertices(points[0].x, points[0].y, points[0].x, 0.0f, points[1].x, points[0].y, points[1].x, 0.0f, &color,
			&color, &color, &color);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
		SetSpriteVertices(points[0].x, (float)*gScreenHeight, points[0].x, points[1].y, points[1].x, (float)*gScreenHeight,
			points[1].x, points[1].y, &color, &color, &color, &color);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
		SetSpriteVertices((float)*gScreenWidth, (float)*gScreenHeight, (float)*gScreenWidth, 0.0f, points[0].x, (float)*gScreenHeight,
			points[0].x, 0.0f, &color, &color, &color, &color);
		RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
	}
	RwD3D9SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
}

void DrawLine(CVector2D const&a, CVector2D const&b, float width, unsigned int color)
{
	CVector2D point[4], shift[2], dir;
		width /= 2.0f;
	dir.x = b.x - a.x;
	dir.y = b.y - a.y;
		float angle = atan2f(dir.y, dir.x);
		shift[0].x = cosf(angle - 1.5707963f) * width;
		shift[0].y = sinf(angle - 1.5707963f) * width;
		shift[1].x = cosf(angle + 1.5707963f) * width;
		shift[1].y = sinf(angle + 1.5707963f) * width;
	point[0].x = a.x + shift[1].x;
	point[0].y = a.y + shift[1].y;
	point[1].x = b.x + shift[1].x;
	point[1].y = b.y + shift[1].y;
	point[2].x = a.x + shift[0].x;
	point[2].y = a.y + shift[0].y;
	point[3].x = b.x + shift[0].x;
	point[3].y = b.y + shift[0].y;
	float oldZ = RwIm2DGetNearScreenZ();
	RwIm2DSetNearScreenZ(oldZ + *gRadarMapZShift);
	SetSpriteVertices(point[0].x, point[0].y, point[1].x, point[1].y, point[2].x, point[2].y, point[3].x, point[3].y, &color,
		&color, &color, &color);
	RwIm2DSetNearScreenZ(oldZ);
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
}


void GetMemoryAddresses()
{
	FindPlayerVehicle = (CVehicle *(__cdecl *)())0x4BC1E0;
	IsPlayerOnAMission = (bool(*)()) 0x44FE30;
	DoPathSearch = (void(__thiscall *)(void *, unsigned char, CVector, int, CVector, CPathNode **, short *, short, CVehicle *, float *, float, int))0x439070;
	RwIm2DGetNearScreenZ = (float(__cdecl *)())0x649B80;
	RwRenderStateSet = (void(__cdecl *)(unsigned int, unsigned int))0x649BA0;
	RwD3D9SetRenderState = (int(__cdecl *)(unsigned int, unsigned int))0x6582A0;
	SetSpriteVertices = (void(__cdecl *)(float, float, float, float, float, float, float, float, unsigned int *, unsigned int *, unsigned int *, unsigned int *))0x5781C0;
	RwIm2DRenderPrimitive = (void(__cdecl *)(unsigned int, RwD3D9Vertex *, unsigned int))0x649C10;
	GetRadarTraceColour = (unsigned int(__cdecl *)(unsigned int, unsigned int))0x4C3050;
	VehicleGetAt = (CVehicle *(__thiscall *)(void *, unsigned int))0x451C70;
	PedGetAt = (CPed *(__thiscall *)(void *, unsigned int))0x451CB0;
	ObjectGetAt = (CObject *(__thiscall *)(void *, unsigned int))0x451C30;
	DrawRadarMap = (void(__cdecl *)())0x4C17C0;
	DrawRadarMask = (void(__cdecl *)())0x4C1A20;
	InitialiseRadar = (void(__cdecl *)())0x4C6200;
	gPathfind = (void *)0x9B6E5C;
	gRadarMapZShift = (float *)0x699530;
	gSpriteVertices = (RwD3D9Vertex *)0x7D4040;
	gScreenWidth = (int *)0x9B48E4;
	gScreenHeight = (int *)0x9B48E8;
	gVehiclePool = (void **)0xA0FDE4;
	gPedPool = (void **)0x97F2AC;
	gObjectPool = (void **)0x94DBE0;
	gRwEngine = (unsigned int *)0x7870C0;
	gRadarRange = (float *)0x974BEC;
	gRadarBlips = (RadarBlip *)0x7D7D38;
	AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x552500;
	PrintString = (void(__cdecl *)(float, float, short *)) 0x551040;
	SetFontStyle = (void(__cdecl *)(int)) 0x54FFE0;
	SetScale = (void(__cdecl *)(float, float)) 0x550230;
	SetColor = (void(__cdecl *)(unsigned int *)) 0x550170;
	SetDropShadowPosition = (void(__cdecl *)(int)) 0x54FF20;
	SetPropOn = (void(__cdecl *)()) 0x550020;
}

void OnMenuDrawing(float x, float y, short *text)
{
    // Call the original function first
    if (pfDrawInMenu) pfDrawInMenu(x, y, text);

    unsigned int color = 0;
    BYTEn(color, 0) = 255; // R
    BYTEn(color, 1) = 77;  // G
    BYTEn(color, 2) = 210; // B
    BYTEn(color, 3) = 255; // A

    SetFontStyle(1);
    SetScale(0.20f * ((float)*gScreenWidth / 640.0f), 0.3f * ((float)*gScreenHeight / 448.0f));
    SetColor(&color);
    SetDropShadowPosition(0);
    SetPropOn();

    short textUni[256];
    AsciiToUnicode("VC GPS mod by CanerKaraca", textUni);

    // Position it at the bottom left, slightly above the bottom edge (to avoid CLEO text)
    float textX = 12.0f * ((float)*gScreenWidth / 640.0f);
    float textY = (float)*gScreenHeight - (25.0f * ((float)*gScreenHeight / 448.0f));

    PrintString(textX, textY, textUni);
}

void Init()
{
	GetMemoryAddresses();
	injector::MakeCALL(0x4C5D4B, ProcessPathfind);
	injector::MakeCALL(0x4C17C5, DrawPathLineMask);
	injector::MakeCALL(0x4A4896, InitialiseRadar);
	injector::MakeNOP(0x4C1D49, 5);

    pfDrawInMenu = (void(__cdecl *)(float, float, short *))injector::MakeCALL(0x49E3D9, OnMenuDrawing).get();
}


using MenuMap_GetScreenCoords_t = void (*)(float, float, float*, float*);
MenuMap_GetScreenCoords_t pMenuMap_GetScreenCoords = nullptr;
unsigned int gPathColor = 0;

bool bCheckedMenuMap = false;
uintptr_t* ppMenuNew = nullptr;
CVector lastMenuTargetPos = {0.0f, 0.0f, 0.0f};
CVector lastMenuPlayerPos = {0.0f, 0.0f, 0.0f};

inline bool operator!=(const CVector& a, const CVector& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

void DrawPathFindLineMenuMap()
{
	if (!pMenuMap_GetScreenCoords) return;

	PathLineInfo info{};
	GetPlaceInfo(&info);

	CVehicle *playerCar = FindPlayerVehicle();

	if (info.targetPoint && playerCar)
	{
		gPathColor = info.color;
		// If target or player position changed, re-calculate path instantly
		if (*info.targetPoint != lastMenuTargetPos || playerCar->m_sCoords.m_sMatrix.pos != lastMenuPlayerPos)
		{
			lastMenuTargetPos = *info.targetPoint;
			lastMenuPlayerPos = playerCar->m_sCoords.m_sMatrix.pos;
			DoPathSearch(gPathfind, static_cast<unsigned char>(ePathNodeType::PATHNODE_VEHICLE_PATH), playerCar->m_sCoords.m_sMatrix.pos, -1, *info.targetPoint, gapPathNodes, &gwPathNodesCount, MAX_POINTS, playerCar, nullptr, 999999.0f, -1);
		}
	}
	else
	{
		gwPathNodesCount = 0;
		lastMenuTargetPos = {0.0f, 0.0f, 0.0f};
		lastMenuPlayerPos = {0.0f, 0.0f, 0.0f};
	}

	if (gwPathNodesCount <= 1) return;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);

	for (int i = 0; i < (int)(gwPathNodesCount - 1); i++)
	{
		CVector2D world1, world2;
		world1.x = gapPathNodes[i]->m_v2dPoint.x * 0.125f;
		world1.y = gapPathNodes[i]->m_v2dPoint.y * 0.125f;
		world2.x = gapPathNodes[i + 1]->m_v2dPoint.x * 0.125f;
		world2.y = gapPathNodes[i + 1]->m_v2dPoint.y * 0.125f;

		CVector2D screen1, screen2;
		pMenuMap_GetScreenCoords(world1.x, world1.y, &screen1.x, &screen1.y);
		pMenuMap_GetScreenCoords(world2.x, world2.y, &screen2.x, &screen2.y);

        // Make the line thicker on the menu map
		DrawLine(screen1, screen2, (LINE_WIDTH / (*gRadarRange)) * 5.0f, gPathColor);
	}
}

PathLineInfo *GetPlaceInfo(PathLineInfo *info)
{
	CPlaceable *entity = nullptr;
	RadarBlip *bestBlip = nullptr;
	CVector blipPos = { 0.0f, 0.0f, 0.0f };
	float distance = 9999800001.99f;
	float newDistance = 0.0f;
	unsigned int color = 0;




	if (!bCheckedMenuMap)
	{
		HMODULE hMenuMap = GetModuleHandleA("MenuMapVC.asi");
		if (hMenuMap)
		{
			using MenuMap_RegisterDrawCallback_t = void (*)(void (*)());
			MenuMap_RegisterDrawCallback_t registerCb = (MenuMap_RegisterDrawCallback_t)GetProcAddress(hMenuMap, "MenuMap_RegisterDrawCallback");
			if (registerCb) {
				registerCb(DrawPathFindLineMenuMap);
				pMenuMap_GetScreenCoords = (MenuMap_GetScreenCoords_t)GetProcAddress(hMenuMap, "MenuMap_GetScreenCoords");
			}

			MODULEINFO moduleInfo;
			if (GetModuleInformation(GetCurrentProcess(), hMenuMap, &moduleInfo, sizeof(moduleInfo)))
			{
				uintptr_t startAddress = (uintptr_t)hMenuMap;
				uintptr_t endAddress = startAddress + moduleInfo.SizeOfImage;

				for (uintptr_t i = startAddress; i < endAddress - 30; i++)
				{
					if (*(BYTE*)i == 0xA1 &&
						*(BYTE*)(i+5) == 0x83 && *(BYTE*)(i+6) == 0xEC &&
						*(BYTE*)(i+8) == 0x85 && *(BYTE*)(i+9) == 0xC0 &&
						*(BYTE*)(i+10) == 0x0F && *(BYTE*)(i+11) == 0x84 &&
						*(BYTE*)(i+16) == 0x83 && *(BYTE*)(i+17) == 0x38 && *(BYTE*)(i+18) == 0x00 &&
						*(BYTE*)(i+19) == 0x0F && *(BYTE*)(i+20) == 0x84 &&
						*(BYTE*)(i+25) == 0x83 && *(BYTE*)(i+26) == 0x78 && *(BYTE*)(i+27) == 0x18 && *(BYTE*)(i+28) == 0x00)
					{
						ppMenuNew = (uintptr_t*)(*(uintptr_t*)(i + 1));
						break;
					}
				}
			}
		}
		bCheckedMenuMap = true;
	}

    if (ppMenuNew && *ppMenuNew)
    {
        int targetBlipIndex = *(int*)(*ppMenuNew + 0x18);
        if (targetBlipIndex == 1)
        {
            CVector* targetBlipWorldPos = (CVector*)(*ppMenuNew + 0x1C);
            if (targetBlipWorldPos->x != 0.0f || targetBlipWorldPos->y != 0.0f)
            {
                CVehicle* playerCar = FindPlayerVehicle();
                if (playerCar)
                {
                    float distSq = GetSquaredDistanceBetweenPoints(playerCar->m_sCoords.m_sMatrix.pos, *targetBlipWorldPos);
                    if (distSq < 225.0f)
                    {
                        if (*(int*)(*ppMenuNew + 0x18) != 0)
                        {
                            PlayFrontEndSound(104, 0); // 104 = PAGER_BEEP, 0 = 0 attenuation (max volume)
                        }
                        *(int*)(*ppMenuNew + 0x18) = 0;
                    }
                    else
                    {
                        BYTEn(info->color, 0) = 255;
                        BYTEn(info->color, 1) = 77;
                        BYTEn(info->color, 2) = 210;
                        BYTEn(info->color, 3) = 255;
                        info->targetPoint = targetBlipWorldPos;
                        return info;
                    }
                }
            }
        }
    }

	if (!IsPlayerOnAMission())
	{
		info->targetPoint = nullptr;
		info->color = 0;
		return info;
	}

	for (RadarBlip *blip = gRadarBlips; blip != &gRadarBlips[75]; blip++)
	{
		if (blip->m_bActive && blip->m_wBlipSprite == 0 && blip->m_wBlipDisplay > 1) // RADAR_SPRITE_NONE and visible on radar (BLIP_DISPLAY_BLIP_ONLY or BLIP_DISPLAY_BOTH)
		{
			if (blip->m_dwBlipType > 0 && blip->m_dwBlipType < 4)
			{
				switch (static_cast<eBlipType>(blip->m_dwBlipType))
				{
				case eBlipType::BLIP_CAR:
					if (gVehiclePool && *gVehiclePool)
						entity = VehicleGetAt(*gVehiclePool, blip->m_dwEntityHandle);
					break;
				case eBlipType::BLIP_PED:
					if (gPedPool && *gPedPool)
					{
						entity = PedGetAt(*gPedPool, blip->m_dwEntityHandle);
						if (entity && IS_PED_IN_CAR(entity))
							entity = GET_PED_CAR(entity);
					}
					break;
				case eBlipType::BLIP_OBJECT:
					if (gObjectPool && *gObjectPool)
						entity = ObjectGetAt(*gObjectPool, blip->m_dwEntityHandle);
					break;
				}
				if (entity)
				{
					blipPos.x = entity->m_sCoords.m_sMatrix.pos.x;
					blipPos.y = entity->m_sCoords.m_sMatrix.pos.y;
					blipPos.z = entity->m_sCoords.m_sMatrix.pos.z;
				}
				else continue;
			}
			else
			{
				blipPos.x = blip->m_vecCurPos.x;
				blipPos.y = blip->m_vecCurPos.y;
				blipPos.z = blip->m_vecCurPos.z;
			}

			if (blipPos.x == 0.0f && blipPos.y == 0.0f)
				continue;

			newDistance = GetSquaredDistanceBetweenPoints(blipPos, *GetCamPos());
			if (newDistance < distance)
			{
				distance = newDistance;
				gBlipBestPos.x = blipPos.x;
				gBlipBestPos.y = blipPos.y;
				gBlipBestPos.z = blipPos.z;
				bestBlip = blip;
			}
		}
	}
	if (bestBlip)
	{
		color = GetRadarTraceColour(bestBlip->m_dwBlipColour, bestBlip->m_bBlipBrightness);
		BYTEn(info->color, 0) = BYTEn(color, 3);
		BYTEn(info->color, 1) = BYTEn(color, 2);
		BYTEn(info->color, 2) = BYTEn(color, 1);
		BYTEn(info->color, 3) = 255;
		info->targetPoint = &gBlipBestPos;
	}
	else
	{
		info->targetPoint = nullptr;
		info->color = 0;
	}
	return info;
}

void ProcessPathfind()
{
	DrawRadarMap();
	PathLineInfo info{};
	CVehicle *playerCar = FindPlayerVehicle();
	if (playerCar)
	{
		GetPlaceInfo(&info);
		gPathColor = info.color;

		if (info.targetPoint)
		{
			DoPathSearch(gPathfind, static_cast<unsigned char>(ePathNodeType::PATHNODE_VEHICLE_PATH), playerCar->m_sCoords.m_sMatrix.pos, -1, *info.targetPoint, gapPathNodes, &gwPathNodesCount, MAX_POINTS, playerCar, nullptr, 999999.0f, -1);
			if (gwPathNodesCount > 1)
			{
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
				for (int i = 0; i < (int)(gwPathNodesCount - 1); i++)
				{
					CVector2D temp, temp2;
					CVector2D radar1, radar2;

					temp.x = gapPathNodes[i]->m_v2dPoint.x * 0.125f;
					temp.y = gapPathNodes[i]->m_v2dPoint.y * 0.125f;
					temp2.x = gapPathNodes[i + 1]->m_v2dPoint.x * 0.125f;
					temp2.y = gapPathNodes[i + 1]->m_v2dPoint.y * 0.125f;

					TransformRealWorldPointToRadarSpace(radar1, temp);
					TransformRealWorldPointToRadarSpace(radar2, temp2);

					if (IsLineInsideRadar(radar1, radar2))
					{
						CVector2D screen1, screen2;
						TransformRadarPointToScreenSpace(screen1, radar1);
						TransformRadarPointToScreenSpace(screen2, radar2);
						DrawLine(screen1, screen2, LINE_WIDTH / (*gRadarRange), info.color);
					}
				}
			}
		}
		else
		{
			gwPathNodesCount = 0;
		}
	}
}


BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}
	return TRUE;
}
