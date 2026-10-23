// Donjon de l'Oie — ce qui s'affiche : maillages, matériaux, caméra, lampes, réglages de lumière et de brume,
// textures et sprites. Le moteur ne dessine rien lui-même : la couche Unreal lit ces objets à chaque image et les
// reproduit (composants de maillage, lampes, caméra, post-traitement).
#pragma once

#include "Objets.h"

namespace Moteur
{
    enum class ShadowCastingMode { Off, On, TwoSided, ShadowsOnly };
    enum class IndexFormat { UInt16, UInt32 };
    enum class BlendMode { Zero = 0, One = 1, DstColor = 2, SrcColor = 3, OneMinusDstColor = 4, SrcAlpha = 5, OneMinusSrcColor = 6, DstAlpha = 7, OneMinusDstAlpha = 8, SrcAlphaSaturate = 9, OneMinusSrcAlpha = 10 };
    enum class CompareFunction { Disabled = 0, Never = 1, Less = 2, Equal = 3, LessEqual = 4, Greater = 5, NotEqual = 6, GreaterEqual = 7, Always = 8 };
    enum class AmbientMode { Skybox = 0, Trilight = 1, Flat = 3, Custom = 4 };

    // ------------------------------------------------------------------ maillages

    class Mesh : public Object
    {
        DD_TYPE_OBJET(Mesh, Object)
    public:
        Liste<Vector3> V;
        Liste<Vector3> N;
        Liste<Color32> C;
        Liste<int> T;
        IndexFormat indexFormat = IndexFormat::UInt16;
        Bounds bounds;

        int vertexCount() const { return V.Count(); }
        void SetVertices(const Liste<Vector3>& v);
        void SetNormals(const Liste<Vector3>& n);
        void SetColors(const Liste<Color32>& c);
        void SetColors(const Liste<Color>& c);
        void SetUVs(int, const Liste<Vector2>&) {}
        void SetTriangles(const Liste<int>& t, int sousMaillage);
        Liste<Vector3> vertices() const { return V; }
        void vertices(const Liste<Vector3>& v);
        Liste<int> triangles() const { return T; }
        void triangles(const Liste<int>& t) { SetTriangles(t, 0); }
        Liste<Color32> colors32() const { return C; }
        void colors32(const Liste<Color32>& c);
        Liste<Vector3> normals() const { return N; }
        void normals(const Liste<Vector3>& n);
        void RecalculateBounds();
        void RecalculateNormals() {}
        void Clear();
        void MarkDynamic() { dynamique = true; }
        void UploadMeshData(bool) {}

        // --- pour la couche d'affichage
        /// Augmente à chaque changement : l'affichage sait quand recopier le maillage.
        int Version() const { return version; }
        bool Dynamique() const { return dynamique; }

    private:
        int version = 0;
        bool dynamique = false;
    };

    class MeshFilter : public Component
    {
        DD_TYPE_OBJET(MeshFilter, Component)
    public:
        Ref<Mesh> sharedMesh;
        Ref<Mesh> mesh() const { return sharedMesh; }
        void mesh(const Ref<Mesh>& m) { sharedMesh = m; }
    };

    /// Couleurs et nombres propres à un objet affiché, sans copier son matériau.
    class MaterialPropertyBlock
    {
    public:
        void SetColor(const string& n, Color c);
        void SetFloat(const string& n, float f);
        void Clear() { couleurs.Clear(); nombres.Clear(); }

        // --- pour la couche d'affichage
        const Dico<string, Color>& Couleurs() const { return couleurs; }
        const Dico<string, float>& Nombres() const { return nombres; }
        bool Vide() const { return couleurs.Count() == 0 && nombres.Count() == 0; }

    private:
        Dico<string, Color> couleurs;
        Dico<string, float> nombres;
    };

    class Shader;

    class Material : public Object
    {
        DD_TYPE_OBJET(Material, Object)
    public:
        /// new Material(shader)
        explicit Material(const Ref<Shader>& s);
        /// new Material(autre) : même shader, même file d'attente de rendu, mêmes propriétés.
        explicit Material(const Ref<Material>& m);

        Ref<Shader> shader;
        int renderQueue = 2000;
        bool enableInstancing = false;
        Color color = Color::white;

        void SetFloat(const string& n, float f);
        void SetInt(const string& n, int i) { SetFloat(n, static_cast<float>(i)); }
        void SetColor(const string& n, Color c);
        void SetVector(const string& n, Vector4 v);
        float GetFloat(const string& n) const;
        Color GetColor(const string& n) const;
        bool HasProperty(const string&) const { return true; }
        void EnableKeyword(const string&) {}
        void DisableKeyword(const string&) {}

        // --- pour la couche d'affichage
        int Version() const { return version; }
        const Dico<string, float>& Nombres() const { return nombres; }
        const Dico<string, Color>& Couleurs() const { return couleurs; }

    private:
        Dico<string, float> nombres;
        Dico<string, Color> couleurs;
        Dico<string, Vector4> vecteurs;
        int version = 0;
    };

    class Renderer : public Component
    {
        DD_TYPE_OBJET(Renderer, Component)
    public:
        bool enabled = true;
        Ref<Material> sharedMaterial;
        Ref<Material> material() const { return sharedMaterial; }
        void material(const Ref<Material>& m) { sharedMaterial = m; }
        Liste<Ref<Material>> sharedMaterials() const { return Liste<Ref<Material>>{sharedMaterial}; }
        void sharedMaterials(const Liste<Ref<Material>>& l) { sharedMaterial = l.Count() > 0 ? l[0] : Ref<Material>(); }
        ShadowCastingMode shadowCastingMode = ShadowCastingMode::On;
        bool receiveShadows = true;
        Bounds bounds;
        int sortingOrder = 0;
        void GetPropertyBlock(MaterialPropertyBlock& b) const
        {
            if (bloc != nullptr) b = *bloc;
        }
        void SetPropertyBlock(const MaterialPropertyBlock& b)
        {
            bloc = Nouveau<MaterialPropertyBlock>(b);
            versionBloc++;
        }

        // --- pour la couche d'affichage
        const MaterialPropertyBlock* Bloc() const { return bloc.get(); }
        int VersionBloc() const { return versionBloc; }

    private:
        Ptr<MaterialPropertyBlock> bloc;
        int versionBloc = 0;
    };

    class MeshRenderer : public Renderer
    {
        DD_TYPE_OBJET(MeshRenderer, Renderer)
    };

    class Shader : public Object
    {
        DD_TYPE_OBJET(Shader, Object)
    public:
        bool isSupported = true;
        /// Les shaders connus : ceux du jeu (DonjonDeLOie/Voxel, /Lueur, /Fx) et les replis d'Unity qu'il demande.
        static Ref<Shader> Find(const string& n);
        static int PropertyToID(const string& n);
    };

    // ------------------------------------------------------------------ caméra, lampes, ambiance

    enum class CameraClearFlags { Skybox = 1, SolidColor = 2, Depth = 3, Nothing = 4 };

    class Camera : public Behaviour
    {
        DD_TYPE_OBJET(Camera, Behaviour)
    public:
        float fieldOfView = 60.0f, nearClipPlane = 0.3f, farClipPlane = 1000.0f, depth = 0.0f;
        bool allowHDR = false, allowMSAA = false, orthographic = false;
        float orthographicSize = 5.0f;
        Color backgroundColor;
        CameraClearFlags clearFlags = CameraClearFlags::Skybox;
        int cullingMask = -1;
        /// La caméra principale : la première dont le GameObject porte l'étiquette « MainCamera ».
        static Ref<Camera> main();
        Vector3 WorldToScreenPoint(Vector3 p) const;
        Ray ScreenPointToRay(Vector3 s) const;
        Rect pixelRect() const { return Rect(0, 0, static_cast<float>(Screen::width), static_cast<float>(Screen::height)); }
        float aspect() const { return static_cast<float>(Screen::width) / static_cast<float>(Screen::height); }
        void aspect(float) {}
    };

    enum class LightType { Spot, Directional, Point, Area };
    enum class LightShadows { None, Hard, Soft };

    class Light : public Behaviour
    {
        DD_TYPE_OBJET(Light, Behaviour)
    public:
        LightType type = LightType::Point;
        Color color = Color::white;
        float intensity = 1.0f, range = 10.0f, spotAngle = 30.0f, shadowStrength = 1.0f, shadowBias = 0.05f, shadowNormalBias = 0.4f, bounceIntensity = 1.0f;
        LightShadows shadows = LightShadows::None;
    };

    enum class FogMode { Linear = 1, Exponential = 2, ExponentialSquared = 3 };

    struct RenderSettings
    {
        static inline AmbientMode ambientMode = AmbientMode::Skybox;
        static inline Color ambientSkyColor, ambientEquatorColor, ambientGroundColor, ambientLight, fogColor;
        static inline float ambientIntensity = 1.0f, fogStartDistance = 0.0f, fogEndDistance = 300.0f, fogDensity = 0.01f, reflectionIntensity = 1.0f;
        static inline bool fog = false;
        static inline FogMode fogMode = FogMode::Linear;
        static inline Ref<Material> skybox;
        static inline Ref<Light> sun;
    };

    struct DynamicGI
    {
        static void UpdateEnvironment() {}
    };

    struct QualitySettings
    {
        static inline float shadowDistance = 50.0f;
        static inline int antiAliasing = 0, vSyncCount = 0;
    };

    // ------------------------------------------------------------------ textures, sprites, polices

    enum class TextureFormat { RGBA32 = 4, ARGB32 = 5, RGB24 = 3, Alpha8 = 1 };
    enum class FilterMode { Point, Bilinear, Trilinear };
    enum class TextureWrapMode { Repeat, Clamp, Mirror };

    class Texture : public Object
    {
        DD_TYPE_OBJET(Texture, Object)
    public:
        FilterMode filterMode = FilterMode::Bilinear;
        TextureWrapMode wrapMode = TextureWrapMode::Repeat;
        int width = 0, height = 0;
    };

    class Texture2D : public Texture
    {
        DD_TYPE_OBJET(Texture2D, Texture)
    public:
        Texture2D(int w, int h);
        Texture2D(int w, int h, TextureFormat, bool) : Texture2D(w, h) {}
        Texture2D(int w, int h, TextureFormat, bool, bool) : Texture2D(w, h) {}
        void SetPixels32(const Liste<Color32>& c);
        void SetPixel(int x, int y, Color c);
        Liste<Color32> GetPixels32() const { return px; }
        void Apply() { version++; }
        void Apply(bool, bool) { version++; }
        static Ref<Texture2D> whiteTexture();

        // --- pour la couche d'affichage : les pixels (ligne 0 en bas, comme Unity) et leur version
        const Liste<Color32>& Pixels() const { return px; }
        int Version() const { return version; }

    private:
        Liste<Color32> px;
        int version = 0;
    };

    enum class SpriteMeshType { FullRect, Tight };

    class Sprite : public Object
    {
        DD_TYPE_OBJET(Sprite, Object)
    public:
        Ref<Texture2D> texture;
        Rect rect;
        Vector4 border;
        static Ref<Sprite> Create(const Ref<Texture2D>& t, Rect r, Vector2 pivot, float pixelsParUnite);
        static Ref<Sprite> Create(const Ref<Texture2D>& t, Rect r, Vector2 pivot, float pixelsParUnite, unsigned int extrusion, SpriteMeshType m, Vector4 bord);
    };

    class Font : public Object
    {
        DD_TYPE_OBJET(Font, Object)
    };

    enum class FontStyle { Normal, Bold, Italic, BoldAndItalic };
    enum class TextAnchor { UpperLeft, UpperCenter, UpperRight, MiddleLeft, MiddleCenter, MiddleRight, LowerLeft, LowerCenter, LowerRight };
    enum class HorizontalWrapMode { Wrap, Overflow };
    enum class VerticalWrapMode { Truncate, Overflow };
}
