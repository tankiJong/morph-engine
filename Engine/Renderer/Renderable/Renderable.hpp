#include "Engine/Core/common.hpp"

class Material;
class Transform;
class Mesh;

class Renderable {
public:
  Renderable() = default;
  Renderable(S<const Material> mat, const Mesh& me, const Transform& trans);
  Renderable(Material& mat, const Mesh& me, const Transform& trans);

  ~Renderable();
  void material(S<const Material> mat);
  void material(Material& mat);
  Material* material();
  const Material* material() const;

  inline const Mesh*& mesh() { return mMesh; };
  inline const Mesh* mesh() const { return mMesh; };

  inline bool useLight() const { return mIsLit; }

  inline const Transform& transform() const { return *mTransform; }
  inline const Transform*& transform() { return mTransform; }
protected:
  S<const Material> mResMaterial = nullptr;
  owner<Material*> mMaterial = nullptr;
  const Mesh* mMesh = nullptr;
  const Transform* mTransform = nullptr;
  bool mIsLit = true;
};

