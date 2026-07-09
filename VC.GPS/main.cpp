#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include <string>
#include <cmath>
#include <cstdint>
#include <array>
#include <numbers>
#include "injector/injector.hpp"

constexpr int MAX_POINTS = 500;

constexpr unsigned int rwRENDERSTATETEXTURERASTER = 1;
constexpr unsigned int rwPRIMTYPETRIFAN = 5;
constexpr unsigned int D3DRS_ALPHAFUNC = 25;
constexpr unsigned int D3DCMP_GREATER = 5;
constexpr float LINE_WIDTH = 1400.0f;

// Memory access templates
template <typename T>
[[nodiscard]] inline T& MemRef(std::uintptr_t address) noexcept {
    return *reinterpret_cast<T*>(address);
}

// Inline templates replacing macros
[[nodiscard]] inline bool IsPedInCar(CPed* ped) noexcept {
    return *reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(ped) + 0x3AC);
}

[[nodiscard]] inline CVehicle* GetPedCar(CPed* ped) noexcept {
    return *reinterpret_cast<CVehicle**>(reinterpret_cast<std::uintptr_t>(ped) + 0x3A8);
}


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

    inline bool operator!=(const RwV3d& other) const noexcept {
        return x != other.x || y != other.y || z != other.z;
    }
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

using CVehicle = CPlaceable;
using CPed = CPlaceable;
using CObject = CPlaceable;
using CVector = RwV3d;
using CVector2D = RwV2d;

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
    CVector        m_vecInitPos;
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

// Compile-time checks for struct sizes
static_assert(sizeof(CPathNode) == 0x14, "CPathNode size mismatch");
static_assert(sizeof(RadarBlip) == 0x38, "RadarBlip size mismatch");
static_assert(sizeof(RwMatrix) == 0x40, "RwMatrix size mismatch");
static_assert(sizeof(CMatrix) == 0x48, "CMatrix size mismatch");

// Global Pointers & Functions Modernization (Eliminating GetMemoryAddresses)
inline auto* const FindPlayerVehicle = reinterpret_cast<CVehicle*(__cdecl*)()>(0x4BC1E0);
inline auto* const IsPlayerOnAMission = reinterpret_cast<bool(*)()>(0x44FE30);
inline auto* const DoPathSearch = reinterpret_cast<void(__thiscall*)(void*, unsigned char, CVector, int, CVector, CPathNode**, short*, short, CVehicle*, float*, float, int)>(0x439070);
inline auto* const RwIm2DGetNearScreenZ = reinterpret_cast<float(__cdecl*)()>(0x649B80);
inline auto* const RwRenderStateSet = reinterpret_cast<void(__cdecl*)(unsigned int, unsigned int)>(0x649BA0);
inline auto* const RwD3D9SetRenderState = reinterpret_cast<int(__cdecl*)(unsigned int, unsigned int)>(0x6582A0);
inline auto* const SetSpriteVertices = reinterpret_cast<void(__cdecl*)(float, float, float, float, float, float, float, float, unsigned int*, unsigned int*, unsigned int*, unsigned int*)>(0x5781C0);
inline auto* const RwIm2DRenderPrimitive = reinterpret_cast<void(__cdecl*)(unsigned int, RwD3D9Vertex*, unsigned int)>(0x649C10);
inline auto* const GetRadarTraceColour = reinterpret_cast<unsigned int(__cdecl*)(unsigned int, unsigned int)>(0x4C3050);
inline auto* const VehicleGetAt = reinterpret_cast<CVehicle*(__thiscall*)(void*, unsigned int)>(0x451C70);
inline auto* const PedGetAt = reinterpret_cast<CPed*(__thiscall*)(void*, unsigned int)>(0x451CB0);
inline auto* const ObjectGetAt = reinterpret_cast<CObject*(__thiscall*)(void*, unsigned int)>(0x451C30);
inline auto* const DrawRadarMap = reinterpret_cast<void(__cdecl*)()>(0x4C17C0);
inline auto* const DrawRadarMask = reinterpret_cast<void(__cdecl*)()>(0x4C1A20);
inline auto* const InitialiseRadar = reinterpret_cast<void(__cdecl*)()>(0x4C6200);

inline void* const gPathfind = reinterpret_cast<void*>(0x9B6E5C);
inline float* const gRadarMapZShift = reinterpret_cast<float*>(0x699530);
inline RwD3D9Vertex* const gSpriteVertices = reinterpret_cast<RwD3D9Vertex*>(0x7D4040);
inline int* const gScreenWidth = reinterpret_cast<int*>(0x9B48E4);
inline int* const gScreenHeight = reinterpret_cast<int*>(0x9B48E8);
inline void** const gVehiclePool = reinterpret_cast<void**>(0xA0FDE4);
inline void** const gPedPool = reinterpret_cast<void**>(0x97F2AC);
inline void** const gObjectPool = reinterpret_cast<void**>(0x94DBE0);
inline unsigned int* const gRwEngine = reinterpret_cast<unsigned int*>(0x7870C0);
inline float* const gRadarRange = reinterpret_cast<float*>(0x974BEC);
inline RadarBlip* const gRadarBlips = reinterpret_cast<RadarBlip*>(0x7D7D38);

inline auto* const AsciiToUnicode = reinterpret_cast<void(__cdecl*)(const char*, short*)>(0x552500);
inline auto* const PrintString = reinterpret_cast<void(__cdecl*)(float, float, short*)>(0x551040);
inline auto* const SetFontStyle = reinterpret_cast<void(__cdecl*)(int)>(0x54FFE0);
inline auto* const SetScale = reinterpret_cast<void(__cdecl*)(float, float)>(0x550230);
inline auto* const SetColor = reinterpret_cast<void(__cdecl*)(unsigned int*)>(0x550170);
inline auto* const SetDropShadowPosition = reinterpret_cast<void(__cdecl*)(int)>(0x54FF20);
inline auto* const SetPropOn = reinterpret_cast<void(__cdecl*)()>(0x550020);

// Global State
inline std::array<CPathNode*, MAX_POINTS> gapPathNodes{};
inline short gwPathNodesCount = 0;
inline CVector gBlipBestPos{};
inline void(*pfDrawInMenu)(float x, float y, short *text) = nullptr;

using MenuMap_GetScreenCoords_t = void (*)(float, float, float*, float*);
inline MenuMap_GetScreenCoords_t pMenuMap_GetScreenCoords = nullptr;
inline unsigned int gPathColor = 0;

inline bool bCheckedMenuMap = false;
inline std::uintptr_t* ppMenuNew = nullptr;
inline CVector lastMenuTargetPos = {0.0f, 0.0f, 0.0f};
inline CVector lastMenuPlayerPos = {0.0f, 0.0f, 0.0f};

// Forward Declarations
void DrawLine(CVector2D const& a, CVector2D const& b, float width, unsigned int color);
void DrawPathLineMask();
void RwIm2DSetNearScreenZ(float z);
void ProcessPathfind();
float GetSquaredDistanceBetweenPoints(CVector const& a, CVector const& b);
bool IsLineInsideRadar(CVector2D const& a, CVector2D const& b);
PathLineInfo* GetPlaceInfo(PathLineInfo* info);

void TransformRadarPointToScreenSpace(CVector2D& a1, CVector2D const& a2)
{
    if (MemRef<std::uint8_t>(0x869665))
    {
        a1.x = (MemRef<float>(0x68FD0C) * MemRef<float>(0x869670) * a2.x + MemRef<float>(0x68FD10) * MemRef<float>(0x869670) + MemRef<float>(0x869674))
            * static_cast<float>(*gScreenWidth)
            * (*MemRef<float*>(0x4C1B1A));
        a1.y = (MemRef<float>(0x869678) - MemRef<float>(0x68FD1C) * MemRef<float>(0x869670) - MemRef<float>(0x68FD18) * MemRef<float>(0x869670) * a2.y)
            * static_cast<float>(*gScreenHeight)
            * (*MemRef<float*>(0x4C1B46));
    }
    else
    {
        float v1 = static_cast<float>(*gScreenWidth) * (*MemRef<float*>(0x4C1B1A)) * MemRef<float>(0x68FD24);
        a1.x = a2.x * v1 * MemRef<float>(0x68FD28) + v1 * MemRef<float>(0x68FD28) + MemRef<float>(0x68FD2C);
        a1.y = MemRef<float>(0x68FD30) * static_cast<float>(*gScreenHeight) * (*MemRef<float*>(0x4C1B46)) * MemRef<float>(0x68FD28)
            + static_cast<float>(*gScreenHeight)
            - MemRef<float>(0x68FD34) * static_cast<float>(*gScreenHeight) * (*MemRef<float*>(0x4C1B46))
            - a2.y * MemRef<float>(0x68FD30) * static_cast<float>(*gScreenHeight) * (*MemRef<float*>(0x4C1B46)) * MemRef<float>(0x68FD28);
    }
}

void TransformRealWorldPointToRadarSpace(CVector2D& a1, CVector2D const& a2)
{
    float v9 = MemRef<float>(0x68FD40) / MemRef<float>(0x974BEC);
    float v10 = (a2.x - MemRef<float>(0x704734)) * v9;
    float v11 = (a2.y - MemRef<float>(0x704738)) * v9;
    a1.x = MemRef<float>(0x70483C) * v11 + MemRef<float>(0x704840) * v10;
    a1.y = MemRef<float>(0x704840) * v11 - MemRef<float>(0x70483C) * v10;
}

CVector* GetCamPos()
{
    return reinterpret_cast<CVector*>(0x7E4688 + 0x7D8);
}

void RwIm2DSetNearScreenZ(float z)
{
    *reinterpret_cast<float*>(*gRwEngine + 0x18) = z;
}

float GetSquaredDistanceBetweenPoints(CVector const& a, CVector const& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return dz * dz + dy * dy + dx * dx;
}

bool IsLineInsideRadar(CVector2D const& a, CVector2D const& b)
{
    return a.x * a.x + a.y * a.y <= 0.8f || b.x * b.x + b.y * b.y <= 1.0f;
}

void DrawPathLineMask()
{
    DrawRadarMask();
    unsigned int color = 0;
    std::array<CVector2D, 2> points{{{1.0f, 1.0f}, {-1.0f, -1.0f}}};

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
    TransformRadarPointToScreenSpace(points[0], points[0]);
    TransformRadarPointToScreenSpace(points[1], points[1]);

    SetSpriteVertices(points[1].x, static_cast<float>(*gScreenHeight), points[1].x, 0.0f, 0.0f, static_cast<float>(*gScreenHeight), 0.0f, 0.0f, &color, &color, &color, &color);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);

    SetSpriteVertices(points[0].x, points[0].y, points[0].x, 0.0f, points[1].x, points[0].y, points[1].x, 0.0f, &color, &color, &color, &color);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);

    SetSpriteVertices(points[0].x, static_cast<float>(*gScreenHeight), points[0].x, points[1].y, points[1].x, static_cast<float>(*gScreenHeight), points[1].x, points[1].y, &color, &color, &color, &color);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);

    SetSpriteVertices(static_cast<float>(*gScreenWidth), static_cast<float>(*gScreenHeight), static_cast<float>(*gScreenWidth), 0.0f, points[0].x, static_cast<float>(*gScreenHeight), points[0].x, 0.0f, &color, &color, &color, &color);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);

    RwD3D9SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
}

void DrawLine(CVector2D const& a, CVector2D const& b, float width, unsigned int color)
{
    std::array<CVector2D, 4> point{};
    std::array<CVector2D, 2> shift{};
    width /= 2.0f;

    CVector2D dir{b.x - a.x, b.y - a.y};
    float angle = std::atan2(dir.y, dir.x);
    constexpr float half_pi = std::numbers::pi_v<float> / 2.0f;

    shift[0].x = std::cos(angle - half_pi) * width;
    shift[0].y = std::sin(angle - half_pi) * width;
    shift[1].x = std::cos(angle + half_pi) * width;
    shift[1].y = std::sin(angle + half_pi) * width;

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

    SetSpriteVertices(point[0].x, point[0].y, point[1].x, point[1].y, point[2].x, point[2].y, point[3].x, point[3].y, &color, &color, &color, &color);
    RwIm2DSetNearScreenZ(oldZ);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, gSpriteVertices, 4);
}

void OnMenuDrawing(float x, float y, short* text)
{
    if (pfDrawInMenu) pfDrawInMenu(x, y, text);

    unsigned int color = 0xFFD24DFF;

    SetFontStyle(1);
    SetScale(0.20f * (static_cast<float>(*gScreenWidth) / 640.0f), 0.3f * (static_cast<float>(*gScreenHeight) / 448.0f));
    SetColor(&color);
    SetDropShadowPosition(0);
    SetPropOn();

    std::array<short, 256> textUni{};
    AsciiToUnicode("VC GPS mod by CanerKaraca", textUni.data());

    float textX = 12.0f * (static_cast<float>(*gScreenWidth) / 640.0f);
    float textY = static_cast<float>(*gScreenHeight) - (25.0f * (static_cast<float>(*gScreenHeight) / 448.0f));

    PrintString(textX, textY, textUni.data());
}

void Init()
{
    injector::MakeCALL(0x4C5D4B, ProcessPathfind);
    injector::MakeCALL(0x4C17C5, DrawPathLineMask);
    injector::MakeCALL(0x4A4896, InitialiseRadar);
    injector::MakeNOP(0x4C1D49, 5);

    pfDrawInMenu = reinterpret_cast<void(__cdecl *)(float, float, short *)>(injector::MakeCALL(0x49E3D9, OnMenuDrawing).get_raw<void>());
}

void DrawPathFindLineMenuMap()
{
    if (!pMenuMap_GetScreenCoords) return;

    PathLineInfo info{};
    GetPlaceInfo(&info);

    CVehicle* playerCar = FindPlayerVehicle();

    if (info.targetPoint && playerCar)
    {
        gPathColor = info.color;
        if (*info.targetPoint != lastMenuTargetPos || playerCar->m_sCoords.m_sMatrix.pos != lastMenuPlayerPos)
        {
            lastMenuTargetPos = *info.targetPoint;
            lastMenuPlayerPos = playerCar->m_sCoords.m_sMatrix.pos;
            DoPathSearch(gPathfind, static_cast<unsigned char>(ePathNodeType::PATHNODE_VEHICLE_PATH), playerCar->m_sCoords.m_sMatrix.pos, -1, *info.targetPoint, gapPathNodes.data(), &gwPathNodesCount, MAX_POINTS, playerCar, nullptr, 999999.0f, -1);
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

    for (int i = 0; i < static_cast<int>(gwPathNodesCount - 1); i++)
    {
        CVector2D world1{gapPathNodes[i]->m_v2dPoint.x * 0.125f, gapPathNodes[i]->m_v2dPoint.y * 0.125f};
        CVector2D world2{gapPathNodes[i + 1]->m_v2dPoint.x * 0.125f, gapPathNodes[i + 1]->m_v2dPoint.y * 0.125f};

        CVector2D screen1{}, screen2{};
        pMenuMap_GetScreenCoords(world1.x, world1.y, &screen1.x, &screen1.y);
        pMenuMap_GetScreenCoords(world2.x, world2.y, &screen2.x, &screen2.y);

        DrawLine(screen1, screen2, (LINE_WIDTH / (*gRadarRange)) * 5.0f, gPathColor);
    }
}

PathLineInfo* GetPlaceInfo(PathLineInfo* info)
{
    CPlaceable* entity = nullptr;
    RadarBlip* bestBlip = nullptr;
    CVector blipPos{0.0f, 0.0f, 0.0f};
    float distance = 9999800001.99f;
    float newDistance = 0.0f;
    unsigned int color = 0;

    if (!bCheckedMenuMap)
    {
        HMODULE hMenuMap = GetModuleHandleA("MenuMapVC.asi");
        if (hMenuMap)
        {
            using MenuMap_RegisterDrawCallback_t = void (*)(void (*)());
            auto registerCb = reinterpret_cast<MenuMap_RegisterDrawCallback_t>(GetProcAddress(hMenuMap, "MenuMap_RegisterDrawCallback"));
            if (registerCb) {
                registerCb(DrawPathFindLineMenuMap);
                pMenuMap_GetScreenCoords = reinterpret_cast<MenuMap_GetScreenCoords_t>(GetProcAddress(hMenuMap, "MenuMap_GetScreenCoords"));
            }

            MODULEINFO moduleInfo{};
            if (GetModuleInformation(GetCurrentProcess(), hMenuMap, &moduleInfo, sizeof(moduleInfo)))
            {
                auto startAddress = reinterpret_cast<std::uintptr_t>(hMenuMap);
                std::uintptr_t endAddress = startAddress + moduleInfo.SizeOfImage;

                for (std::uintptr_t i = startAddress; i < endAddress - 30; i++)
                {
                    if (MemRef<std::uint8_t>(i) == 0xA1 &&
                        MemRef<std::uint8_t>(i+5) == 0x83 && MemRef<std::uint8_t>(i+6) == 0xEC &&
                        MemRef<std::uint8_t>(i+8) == 0x85 && MemRef<std::uint8_t>(i+9) == 0xC0 &&
                        MemRef<std::uint8_t>(i+10) == 0x0F && MemRef<std::uint8_t>(i+11) == 0x84 &&
                        MemRef<std::uint8_t>(i+16) == 0x83 && MemRef<std::uint8_t>(i+17) == 0x38 && MemRef<std::uint8_t>(i+18) == 0x00 &&
                        MemRef<std::uint8_t>(i+19) == 0x0F && MemRef<std::uint8_t>(i+20) == 0x84 &&
                        MemRef<std::uint8_t>(i+25) == 0x83 && MemRef<std::uint8_t>(i+26) == 0x78 && MemRef<std::uint8_t>(i+27) == 0x18 && MemRef<std::uint8_t>(i+28) == 0x00)
                    {
                        ppMenuNew = reinterpret_cast<std::uintptr_t*>(MemRef<std::uintptr_t>(i + 1));
                        break;
                    }
                }
            }
        }
        bCheckedMenuMap = true;
    }

    if (ppMenuNew && *ppMenuNew)
    {
        int targetBlipIndex = MemRef<int>(*ppMenuNew + 0x18);
        if (targetBlipIndex == 1)
        {
            auto* targetBlipWorldPos = reinterpret_cast<CVector*>(*ppMenuNew + 0x1C);
            if (targetBlipWorldPos->x != 0.0f || targetBlipWorldPos->y != 0.0f)
            {
                CVehicle* playerCar = FindPlayerVehicle();
                if (playerCar)
                {
                    float distSq = GetSquaredDistanceBetweenPoints(playerCar->m_sCoords.m_sMatrix.pos, *targetBlipWorldPos);
                    if (distSq < 225.0f)
                    {
                        MemRef<int>(*ppMenuNew + 0x18) = 0;
                    }
                    else
                    {
                        info->color = 0xFFD24DFF;
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

    for (RadarBlip* blip = gRadarBlips; blip != &gRadarBlips[75]; ++blip)
    {
        if (blip->m_bActive && blip->m_wBlipSprite == 0 && blip->m_wBlipDisplay > 1)
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
                        if (entity && IsPedInCar(entity))
                            entity = GetPedCar(entity);
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
                gBlipBestPos = blipPos;
                bestBlip = blip;
            }
        }
    }
    if (bestBlip)
    {
        color = GetRadarTraceColour(bestBlip->m_dwBlipColour, bestBlip->m_bBlipBrightness);
        info->color = ((color >> 24) & 0xFF) |
                      (((color >> 16) & 0xFF) << 8) |
                      (((color >> 8) & 0xFF) << 16) |
                      (255 << 24);
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
    CVehicle* playerCar = FindPlayerVehicle();

    if (playerCar)
    {
        GetPlaceInfo(&info);
        gPathColor = info.color;

        if (info.targetPoint)
        {
            DoPathSearch(gPathfind, static_cast<unsigned char>(ePathNodeType::PATHNODE_VEHICLE_PATH), playerCar->m_sCoords.m_sMatrix.pos, -1, *info.targetPoint, gapPathNodes.data(), &gwPathNodesCount, MAX_POINTS, playerCar, nullptr, 999999.0f, -1);
            if (gwPathNodesCount > 1)
            {
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
                for (int i = 0; i < static_cast<int>(gwPathNodesCount - 1); i++)
                {
                    CVector2D temp{gapPathNodes[i]->m_v2dPoint.x * 0.125f, gapPathNodes[i]->m_v2dPoint.y * 0.125f};
                    CVector2D temp2{gapPathNodes[i + 1]->m_v2dPoint.x * 0.125f, gapPathNodes[i + 1]->m_v2dPoint.y * 0.125f};

                    CVector2D radar1{}, radar2{};
                    TransformRealWorldPointToRadarSpace(radar1, temp);
                    TransformRealWorldPointToRadarSpace(radar2, temp2);

                    if (IsLineInsideRadar(radar1, radar2))
                    {
                        CVector2D screen1{}, screen2{};
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
