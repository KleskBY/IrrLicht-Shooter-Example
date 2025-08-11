#include "lightgrid.h"
#include "materials.h"
#include <unordered_map>
#include <fstream>
#include "scene.h"

LightGrid g_LightGrid;



// Helper: barycentric coordinates
static core::vector3df ComputeBarycentric(const core::vector3df& p, const core::vector3df& a, const core::vector3df& b, const core::vector3df& c)
{
    core::vector3df v0 = b - a;
    core::vector3df v1 = c - a;
    core::vector3df v2 = p - a;

    float d00 = v0.dotProduct(v0);
    float d01 = v0.dotProduct(v1);
    float d11 = v1.dotProduct(v1);
    float d20 = v2.dotProduct(v0);
    float d21 = v2.dotProduct(v1);
    float denom = d00 * d11 - d01 * d01;

    if (fabs(denom) < 1e-6f) return core::vector3df(-1, -1, -1); // Degenerate

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return core::vector3df(u, v, w);
}

LightSample GetLightColorAtPoint(const core::vector3df& pos, IMesh* mesh)
{
    LightSample result = { video::SColor(255,0,0,0), 0.0f };
    if (!mesh) return result;

    float bestDistSq = FLT_MAX;  // nearest surface

    IMesh* subMesh = mesh;
    for (u32 b = 0; b < subMesh->getMeshBufferCount(); ++b)
    {
        IMeshBuffer* buffer = subMesh->getMeshBuffer(b);
        if (!buffer) continue;

        auto* verts = (video::S3DVertex2TCoords*)buffer->getVertices();
        const u16* indices = buffer->getIndices();
        u32 indexCount = buffer->getIndexCount();

        ITexture* lightmap = buffer->getMaterial().getTexture(1);
        if (!lightmap) continue;

        core::dimension2du size = lightmap->getSize();
        void* pixels = lightmap->lock(video::ETLM_READ_ONLY);
        if (!pixels) continue;

        u32* pixelData = (u32*)pixels;

        for (u32 t = 0; t < indexCount; t += 3)
        {
            const core::vector3df& v0 = verts[indices[t + 0]].Pos;
            const core::vector3df& v1 = verts[indices[t + 1]].Pos;
            const core::vector3df& v2 = verts[indices[t + 2]].Pos;

            // Project point onto triangle plane
            core::triangle3df tri(v0, v1, v2);
            core::vector3df nearest = tri.closestPointOnTriangle(pos);
            float distSq = pos.getDistanceFromSQ(nearest);
            if (distSq >= bestDistSq) continue;

            auto bary = ComputeBarycentric(nearest, v0, v1, v2);
            if (bary.X < 0 || bary.Y < 0 || bary.Z < 0) continue;

            // Interpolate lightmap UV
            core::vector2df lm0 = verts[indices[t + 0]].TCoords2;
            core::vector2df lm1 = verts[indices[t + 1]].TCoords2;
            core::vector2df lm2 = verts[indices[t + 2]].TCoords2;

            core::vector2df lmUV =
                lm0 * bary.X + lm1 * bary.Y + lm2 * bary.Z;

            s32 px = (s32)(lmUV.X * size.Width);
            s32 py = (s32)(lmUV.Y * size.Height);
            px = core::clamp(px, 0, (s32)size.Width - 1);
            py = core::clamp(py, 0, (s32)size.Height - 1);

            u32 colorVal = pixelData[py * size.Width + px];
            video::SColor color(colorVal);

            result.color = color;
            result.brightness = (color.getRed() + color.getGreen() + color.getBlue()) / 3.0f;
            bestDistSq = distSq;
        }

        lightmap->unlock();
    }

    return result;
}

void BakeLightGrid(IMesh* mesh, float cellSize)
{
    // 1. World bounds
    core::aabbox3df box = mesh->getBoundingBox();
    std::cout << box.MinEdge.X << " " << box.MinEdge.Y << " " << box.MinEdge.Z << std::endl;
    std::cout << box.MaxEdge.X << " " << box.MaxEdge.Y << " " << box.MaxEdge.Z << std::endl;

    core::vector3df extent = box.getExtent();
    core::vector3di size((int)ceil(extent.X / cellSize), (int)ceil(extent.Y / cellSize), (int)ceil(extent.Z / cellSize));

    g_LightGrid.bounds = box;
    g_LightGrid.size = size;
    g_LightGrid.cellSize = cellSize;
    g_LightGrid.data.resize(size.X * size.Y * size.Z);

    // 2. Fill grid
    for (int z = 0; z < size.Z; z++)
        for (int y = 0; y < size.Y; y++)
            for (int x = 0; x < size.X; x++)
            {
                core::vector3df samplePos(
                    box.MinEdge.X + x * cellSize + cellSize * 0.5f,
                    box.MinEdge.Y + y * cellSize + cellSize * 0.5f,
                    box.MinEdge.Z + z * cellSize + cellSize * 0.5f);

                // Use existing per-point sampler (slow, only here)
                LightSample s = GetLightColorAtPoint(samplePos, mesh);

                g_LightGrid.data[x + y * size.X + z * size.X * size.Y] = s.color;
            }
}


video::SColor SampleLightGrid(const core::vector3df& pos)
{
    if (g_LightGrid.data.empty() || g_LightGrid.size.X == 0 || g_LightGrid.size.Y == 0 || g_LightGrid.size.Z == 0)
    {
        return video::SColor(255, 1, 0, 0); // fallback: black
        std::cout << "g_LightGrid.data.empty()" << std::endl;
    }

    const auto& grid = g_LightGrid;
    const auto& box = grid.bounds;

    // Convert world pos → local grid coords
    core::vector3df local = pos - box.MinEdge;
    local /= grid.cellSize;

    int x = (int)floor(local.X);
    int y = (int)floor(local.Y);
    int z = (int)floor(local.Z);

    float fx = local.X - x;
    float fy = local.Y - y;
    float fz = local.Z - z;

    auto getColor = [&](int xi, int yi, int zi)->video::SColor {
        xi = core::clamp(xi, 0, grid.size.X - 1);
        yi = core::clamp(yi, 0, grid.size.Y - 1);
        zi = core::clamp(zi, 0, grid.size.Z - 1);
        return grid.data[xi + yi * grid.size.X + zi * grid.size.X * grid.size.Y];
        };

    // Fetch 8 corners
    SColor c000 = getColor(x, y, z);
    SColor c100 = getColor(x + 1, y, z);
    SColor c010 = getColor(x, y + 1, z);
    SColor c110 = getColor(x + 1, y + 1, z);
    SColor c001 = getColor(x, y, z + 1);
    SColor c101 = getColor(x + 1, y, z + 1);
    SColor c011 = getColor(x, y + 1, z + 1);
    SColor c111 = getColor(x + 1, y + 1, z + 1);

    auto lerp = [](int a, int b, float t) {return a + (b - a) * t; };

    // Interpolate along X
    int r00 = lerp(c000.getRed(), c100.getRed(), fx);
    int g00 = lerp(c000.getGreen(), c100.getGreen(), fx);
    int b00 = lerp(c000.getBlue(), c100.getBlue(), fx);

    int r10 = lerp(c010.getRed(), c110.getRed(), fx);
    int g10 = lerp(c010.getGreen(), c110.getGreen(), fx);
    int b10 = lerp(c010.getBlue(), c110.getBlue(), fx);

    int r01 = lerp(c001.getRed(), c101.getRed(), fx);
    int g01 = lerp(c001.getGreen(), c101.getGreen(), fx);
    int b01 = lerp(c001.getBlue(), c101.getBlue(), fx);

    int r11 = lerp(c011.getRed(), c111.getRed(), fx);
    int g11 = lerp(c011.getGreen(), c111.getGreen(), fx);
    int b11 = lerp(c011.getBlue(), c111.getBlue(), fx);

    // Interpolate along Y
    int r0 = lerp(r00, r10, fy);
    int g0 = lerp(g00, g10, fy);
    int b0 = lerp(b00, b10, fy);

    int r1 = lerp(r01, r11, fy);
    int g1 = lerp(g01, g11, fy);
    int b1 = lerp(b01, b11, fy);

    // Interpolate along Z
    int r = lerp(r0, r1, fz);
    int g = lerp(g0, g1, fz);
    int b = lerp(b0, b1, fz);

    int brightness = r + g + b;
    return video::SColor(255, r + brightness, g + brightness, b + brightness);
}



















struct LightmapCache {
    video::ITexture* texture;
    video::IImage* image;
};

std::vector<LightmapCache> lightmapCache;

void DisplayLightmaps(scene::IMesh* mesh)
{
    for (u32 i = 0; i < mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);

        // Cast to 2TCoords type (has two UV sets)
        scene::SMeshBufferLightMap* lmBuf = (scene::SMeshBufferLightMap*)buffer;

        // Access vertices
        u32 vCount = lmBuf->Vertices.size();
        for (u32 v = 0; v < vCount; ++v)
        {
            // Copy LightMap UVs into primary UVs
            lmBuf->Vertices[v].TCoords = lmBuf->Vertices[v].TCoords2;
        }

        // Use only lightmap texture
        buffer->getMaterial().setTexture(0, buffer->getMaterial().getTexture(1));
        //buffer->getMaterial().setTexture(1, nullptr);
        buffer->getMaterial().MaterialType = video::EMT_SOLID;
    }
}

void CacheLightmaps(scene::IMesh* mesh) 
{
    lightmapCache.clear();
    //DisplayLightmaps(mesh);
    for (u32 i = 0; i < mesh->getMeshBufferCount(); ++i) 
    {
        auto& mat = mesh->getMeshBuffer(i)->getMaterial();
        video::ITexture* lmTex = mat.getTexture(1); // lightmap layer

        if (!lmTex) continue;

        // Check if already cached
        bool exists = false;
        for (auto& c : lightmapCache)
            if (c.texture == lmTex) { exists = true; break; }
        if (exists) continue;

        // Lock and store image
        video::IImage* img = driver->createImageFromData(lmTex->getColorFormat(), lmTex->getSize(), lmTex->lock(), false);

        lmTex->unlock();

        lightmapCache.push_back({ lmTex, img });
    }
}

video::SColor GetLightmapColorAtRayHit(IMesh* SceneMesh, vector3df start)
{
    // Cast ray downward
    core::vector3df end = start + core::vector3df(0, -1000, 0);

    core::vector3df hitPoint;
    core::triangle3df tri;
    scene::ISceneNode* hitNode = nullptr;

    if (!collMan->getCollisionPoint(core::line3df(start, end), meta, hitPoint, tri, hitNode))
        return video::SColor(255, 255, 255, 255); // default white

    // Find triangle in mesh buffers
    for (u32 i = 0; i < SceneMesh->getMeshBufferCount(); ++i)
    {
        auto* buffer = SceneMesh->getMeshBuffer(i);
        auto* vBuf = (scene::SMeshBufferLightMap*)buffer;

        u16* indices = vBuf->getIndices();
        u32 icount = vBuf->getIndexCount();

        for (u32 j = 0; j < icount; j += 3)
        {
            core::vector3df v0 = vBuf->Vertices[indices[j + 0]].Pos;
            core::vector3df v1 = vBuf->Vertices[indices[j + 1]].Pos;
            core::vector3df v2 = vBuf->Vertices[indices[j + 2]].Pos;

            // Compare with hit triangle (approx)
            if (tri.isPointInside(v0) || tri.isPointInside(v1) || tri.isPointInside(v2))
            {
                // Compute barycentric coordinates
                core::vector3df p = hitPoint;
                core::vector3df v0v1 = v1 - v0;
                core::vector3df v0v2 = v2 - v0;
                core::vector3df v0p = p - v0;

                float d00 = v0v1.dotProduct(v0v1);
                float d01 = v0v1.dotProduct(v0v2);
                float d11 = v0v2.dotProduct(v0v2);
                float d20 = v0p.dotProduct(v0v1);
                float d21 = v0p.dotProduct(v0v2);
                float denom = d00 * d11 - d01 * d01;

                float v = (d11 * d20 - d01 * d21) / denom;
                float w = (d00 * d21 - d01 * d20) / denom;
                float u = 1.0f - v - w;

                // Interpolate lightmap UV (TCoords2)
                core::vector2df uv =
                    vBuf->Vertices[indices[j + 0]].TCoords2 * u +
                    vBuf->Vertices[indices[j + 1]].TCoords2 * v +
                    vBuf->Vertices[indices[j + 2]].TCoords2 * w;

                // Scale to pixel coordinates
                video::ITexture* lmTex = buffer->getMaterial().getTexture(1);
                if (!lmTex) return video::SColor(255, 255, 255, 255);

                // Find cached image
                video::IImage* img = nullptr;
                for (auto& c : lightmapCache)
                    if (c.texture == lmTex) { img = c.image; break; }

                if (!img) return video::SColor(255, 255, 255, 255);

                core::dimension2du size = img->getDimension();
                s32 px = (s32)(uv.X * size.Width);
                s32 py = (s32)(uv.Y * size.Height);

                // Clamp coordinates
                px = core::clamp(px, 0, (s32)size.Width - 1);
                py = core::clamp(py, 0, (s32)size.Height - 1);

                return img->getPixel(px, py);
            }
        }
    }

    return video::SColor(255, 255, 255, 255); // fallback
}




