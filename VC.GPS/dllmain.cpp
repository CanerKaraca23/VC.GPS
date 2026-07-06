#include <Windows.h>
#include <string>
#include "..\common\common.h"
#include "..\common\injector\injector.hpp"

void GetMemoryAddresses()
{
	FindPlayerVehicle = (CVehicle *(__cdecl *)())0x4BC1E0; 
	IsPlayerOnAMission = (bool(*)()) 0x44FE30;
	DoPathSearch = (void(__thiscall *)(void *, unsigned char, CVector, int, CVector, CPathNode **, short *, short, CVehicle *, float *, float, int))0x439070; 
	//TransformRealWorldPointToRadarSpace = (void(__cdecl *)(CVector2D &, CVector2D const&))0x4A50D0;
	//TransformRadarPointToScreenSpace = (void(__cdecl *)(CVector2D &, CVector2D const&))0x4A5040;
	RwIm2DGetNearScreenZ = (float(__cdecl *)())0x649B80;
	RwRenderStateSet = (void(__cdecl *)(unsigned int, unsigned int))0x649BA0;
	RwD3D9SetRenderState = (int(__cdecl *)(unsigned int, unsigned int))0x6582A0;
	SetSpriteVertices = (void(__cdecl *)(float, float, float, float, float, float, float, float, unsigned int *, unsigned int *, unsigned int *, unsigned int *))0x5781C0;
	RwIm2DRenderPrimitive = (void(__cdecl *)(unsigned int, RwD3D9Vertex *, unsigned int))0x649C10;
	GetRadarTraceColour = (unsigned int(__cdecl *)(unsigned int, unsigned int))0x4C3050;
	ShowRadarTrace = (void(__cdecl *)(float, float, unsigned int, unsigned char, unsigned char, unsigned char, unsigned char, unsigned __int8)) 0x4C32F0;
	VehicleGetAt = (CPed *(__thiscall *)(void *, unsigned int))0x451C70;
	PedGetAt = (CVehicle *(__thiscall *)(void *, unsigned int))0x451CB0;
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
	gCurrLevel = (eLevelName *)0xA0D9AC;
	gAudio = (void *)0xA10B8A;
	g_TimeMs = (unsigned int *)0x974B2C;
	GetText = (wchar_t *(__thiscall *)(int, char *))0x584F30;
	TheText = 0x94B220;
	AsciiToUnicode = (void(__cdecl *)(const char *, short *)) 0x552500;
	PrintString = (void(__cdecl *)(float, float, short *)) 0x551040;
	SetFontStyle = (void(__cdecl *)(int)) 0x54FFE0;
	SetScale = (void(__cdecl *)(float, float)) 0x550230;
	SetColor = (void(__cdecl *)(unsigned int *)) 0x550170;
	SetJustifyOn = (void(__cdecl *)()) 0x550150;
	SetDropShadowPosition = (void(__cdecl *)(int)) 0x54FF20;
	SetPropOn = (void(__cdecl *)()) 0x550020;
    SetLeftJustifyOn = (void(__cdecl *)()) 0x5500A0; // Found via CLEO source matching SetLeftJustifyOn logic.
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
    SetScale(0.4f * ((float)*gScreenWidth / 640.0f), 0.6f * ((float)*gScreenHeight / 448.0f));
    SetColor(&color);
    SetDropShadowPosition(1);
    SetPropOn();

    short textUni[256];
    AsciiToUnicode("VC GPS mod by CanerKaraca", textUni);

    // Position it at the bottom left, slightly above the bottom edge (to avoid CLEO text)
    float textX = 10.0f * ((float)*gScreenWidth / 640.0f);
    float textY = (float)*gScreenHeight - (35.0f * ((float)*gScreenHeight / 448.0f));

    PrintString(textX, textY, textUni);
}

void Init()
{
	GetMemoryAddresses();
	injector::MakeCALL(0x4C5D4B, ProcessPathfind);
	injector::MakeCALL(0x4C17C5, DrawPathLineMask);
	injector::MakeCALL(0x4A4896, InitialiseGps);
	injector::MakeNOP(0x4C1D49, 5);

    pfDrawInMenu = (void(__cdecl *)(float, float, short *))injector::MakeCALL(0x49E3D9, OnMenuDrawing).get();
}


void InitialiseGps()
{
	InitialiseRadar();
	gCurrentGpsMode = RADAR_SPRITE_NONE;
	gGpsTextTimer = 0;
	gGpsAudioTimer = 0;
}





#include <psapi.h>
#pragma comment(lib, "psapi.lib")

typedef void (*MenuMap_GetScreenCoords_t)(float, float, float*, float*);
MenuMap_GetScreenCoords_t pMenuMap_GetScreenCoords = NULL;
unsigned int gPathColor = 0;

bool bCheckedMenuMap = false;
uintptr_t* ppMenuNew = NULL;
CVector lastMenuTargetPos = {0.0f, 0.0f, 0.0f};

void DrawPathFindLineMenuMap()
{
	if (!pMenuMap_GetScreenCoords) return;

    if (ppMenuNew && *ppMenuNew)
    {
        int targetBlipIndex = *(int*)(*ppMenuNew + 0x18);
        if (targetBlipIndex == 1)
        {
            CVector* targetBlipWorldPos = (CVector*)(*ppMenuNew + 0x1C);
            if (targetBlipWorldPos->x != 0.0f || targetBlipWorldPos->y != 0.0f)
            {
                CVehicle *playerCar = FindPlayerVehicle();
                if (playerCar)
                {
                    // If target changed, re-calculate path instantly
                    if (targetBlipWorldPos->x != lastMenuTargetPos.x || targetBlipWorldPos->y != lastMenuTargetPos.y || targetBlipWorldPos->z != lastMenuTargetPos.z)
                    {
                        lastMenuTargetPos = *targetBlipWorldPos;
                        DoPathSearch(gPathfind, PATHNODE_VEHICLE_PATH, playerCar->m_sCoords.m_sMatrix.pos, -1, *targetBlipWorldPos, gapPathNodes, &gwPathNodesCount, MAX_POINTS, playerCar, NULL, 999999.0f, -1);

                        BYTEn(gPathColor, 0) = 255;
                        BYTEn(gPathColor, 1) = 77;
                        BYTEn(gPathColor, 2) = 210;
                        BYTEn(gPathColor, 3) = 255;
                    }
                }
            }
            else
            {
                gwPathNodesCount = 0;
                lastMenuTargetPos = {0.0f, 0.0f, 0.0f};
            }
        }
        else
        {
            // Waypoint removed, clear path instantly
            gwPathNodesCount = 0;
            lastMenuTargetPos = {0.0f, 0.0f, 0.0f};
        }
    }

	if (gwPathNodesCount <= 1) return;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

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
	CPlaceable *entity = NULL;
	RadarBlip *bestBlip = NULL;
	CVector blipPos = { 0.0f, 0.0f, 0.0f };
	float distance = 9999800001.99f;
	float newDistance = 0.0f;
	unsigned int color = 0;
	int point = 0;




	if (!bCheckedMenuMap)
	{
		HMODULE hMenuMap = GetModuleHandleA("MenuMapVC.asi");
		if (hMenuMap)
		{
			typedef void (*MenuMap_RegisterDrawCallback_t)(void (*)());
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
		info->targetPoint = NULL;
		info->color = 0;
		return info;
	}

	for (RadarBlip *blip = gRadarBlips; blip != &gRadarBlips[75]; blip++)
	{
		if (blip->m_bActive && blip->m_wBlipSprite == RADAR_SPRITE_NONE)
		{
			if (blip->m_dwBlipType > 0 && blip->m_dwBlipType < 4)
			{
				switch (blip->m_dwBlipType)
				{
				case BLIP_CAR:
					entity = VehicleGetAt(*gVehiclePool, blip->m_dwEntityHandle);
					break;
				case BLIP_PED:
					entity = PedGetAt(*gPedPool, blip->m_dwEntityHandle);
					if (entity && IS_PED_IN_CAR(entity))
						entity = GET_PED_CAR(entity);
					break;
				case BLIP_OBJECT:
					entity = ObjectGetAt(*gObjectPool, blip->m_dwEntityHandle);
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
		info->targetPoint = NULL;
		info->color = 0;
	}
	return info;
}

void ProcessPathfind()
{
	DrawRadarMap();
	PathLineInfo info = {0};
	CVehicle *playerCar = FindPlayerVehicle();
	if (playerCar)
	{
		GetPlaceInfo(&info);
		gPathColor = info.color;

		if (info.targetPoint)
		{
			DoPathSearch(gPathfind, PATHNODE_VEHICLE_PATH, playerCar->m_sCoords.m_sMatrix.pos, -1, *info.targetPoint, gapPathNodes, &gwPathNodesCount, MAX_POINTS, playerCar, NULL, 999999.0f, -1);
			if (gwPathNodesCount > 1)
			{
				RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
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