
#include "Precompiled.h"
#include "SoftRenderer.h"
#include <random>
using namespace CK::DDD;

// ����� �׸��� �Լ�
void SoftRenderer::DrawGizmo3D()
{
	auto& r = GetRenderer();
	const GameEngine& g = Get3DGameEngine();

	// �� ����� �׸���
	std::vector<Vertex3D> viewGizmo = {
		Vertex3D(Vector4(Vector3::Zero)),
		Vertex3D(Vector4(Vector3::UnitX * _GizmoUnitLength)),
		Vertex3D(Vector4(Vector3::UnitY * _GizmoUnitLength)),
		Vertex3D(Vector4(Vector3::UnitZ * _GizmoUnitLength)),
	};

	Matrix4x4 viewMatRotationOnly = g.GetMainCamera().GetViewMatrixRotationOnly();
	VertexShader3D(viewGizmo, viewMatRotationOnly);

	// �� �׸���
	Vector2 v0 = viewGizmo[0].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v1 = viewGizmo[1].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v2 = viewGizmo[2].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v3 = viewGizmo[3].Position.ToVector2() + _GizmoPositionOffset;
	r.DrawLine(v0, v1, LinearColor::Red);
	r.DrawLine(v0, v2, LinearColor::Green);
	r.DrawLine(v0, v3, LinearColor::Blue);
}

// ���� ������Ʈ �̸�
const std::string SunGo("Sun");
const std::string EarthGo("Earth");
const std::string MoonGo("Moon");

// �� �ε�
void SoftRenderer::LoadScene3D()
{
	GameEngine& g = Get3DGameEngine();

	static const float sunScale = 100.f;
	static const float earthScale = 40.f;
	static const float moonScale = 20.f;
	static const Vector3 earthOffset(5.f, 0.0f, 0.f);
	static const Vector3 moonOffset(3.f, 0.0f, 0.f);

	// �¾�
	GameObject& goSun = g.CreateNewGameObject(SunGo);
	goSun.SetMesh(GameEngine::CubeMesh);
	goSun.SetColor(LinearColor::White);
	goSun.GetTransform().SetWorldScale(Vector3::One * sunScale);

	// ����
	GameObject& goEarth = g.CreateNewGameObject(EarthGo);
	goEarth.SetMesh(GameEngine::CubeMesh);
	goEarth.GetTransform().SetWorldScale(Vector3::One * earthScale);
	goEarth.SetColor(LinearColor::White);
	goEarth.SetParent(goSun);
	goEarth.GetTransform().SetLocalPosition(earthOffset);

	// ��
	GameObject& goMoon = g.CreateNewGameObject(MoonGo);
	goMoon.SetMesh(GameEngine::CubeMesh);
	goMoon.GetTransform().SetWorldPosition(moonOffset);
	goMoon.GetTransform().SetWorldScale(Vector3::One * moonScale);
	goMoon.SetColor(LinearColor::White);
	goMoon.SetParent(goEarth);
	goMoon.GetTransform().SetLocalPosition(moonOffset);

	// ī�޶� ����
	CameraObject& mainCamera = g.GetMainCamera();
	mainCamera.GetTransform().SetWorldPosition(Vector3(0.f, 800.f, 1000.f));
	mainCamera.SetLookAtRotation(goSun);
}

// �ǽ��� ���� ����

// ���� ������ ����ϴ� �Լ�
void SoftRenderer::Update3D(float InDeltaSeconds)
{
	// ���� �������� ����ϴ� ��� �� �ֿ� ���۷���
	GameEngine& g = Get3DGameEngine();
	const InputManager& input = g.GetInputManager();

	// ���� ������ ���� ����
	static float fovSpeed = 100.f;
	static float minFOV = 15.f;
	static float maxFOV = 150.f;
	static float moveSpeed = 500.f;
	static float rotateSpeedSun = 40.f;
	static float rotateSpeedEarth = 120.f;
	static float rotateSpeedMoon = 48.f;

	// �Է¿� ���� ī�޶� �þ߰��� ����
	CameraObject& camera = g.GetMainCamera();
	float deltaFOV = input.GetAxis(InputAxis::WAxis) * fovSpeed * InDeltaSeconds;
	camera.SetFOV(Math::Clamp(camera.GetFOV() + deltaFOV, minFOV, maxFOV));

	//
	GameObject& goSun = g.GetGameObject(SunGo);
	GameObject& goEarth = g.GetGameObject(EarthGo);
	GameObject& goMoon = g.GetGameObject(MoonGo);

	//
	goSun.GetTransform().AddLocalYawRotation(rotateSpeedSun * InDeltaSeconds);
	goEarth.GetTransform().AddLocalYawRotation(rotateSpeedEarth * InDeltaSeconds);
	goMoon.GetTransform().AddLocalYawRotation(rotateSpeedMoon * InDeltaSeconds);

	//
	Vector3 inputVector = Vector3(input.GetAxis(InputAxis::XAxis),
		input.GetAxis(InputAxis::YAxis),
		input.GetAxis(InputAxis::ZAxis)).GetNormalize();
	camera.GetTransform().AddWorldPosition(inputVector * moveSpeed * InDeltaSeconds);
	camera.SetLookAtRotation(goSun);
}

// �ִϸ��̼� ������ ����ϴ� �Լ�
void SoftRenderer::LateUpdate3D(float InDeltaSeconds)
{
	// �ִϸ��̼� �������� ����ϴ� ��� �� �ֿ� ���۷���
	GameEngine& g = Get3DGameEngine();

	// �ִϸ��̼� ������ ���� ����

}

// ������ ������ ����ϴ� �Լ�
void SoftRenderer::Render3D()
{
	// ������ �������� ����ϴ� ��� �� �ֿ� ���۷���
	const GameEngine& g = Get3DGameEngine();
	auto& r = GetRenderer();
	const CameraObject& mainCamera = g.GetMainCamera();

	// ��濡 ����� �׸���
	DrawGizmo3D();

	// ������ ������ ���� ����
	const Matrix4x4 pvMatrix = mainCamera.GetPerspectiveViewMatrix();

	// ����ü �ø� �׽�Ʈ�� ���� ��� ����
	size_t totalObjects = g.GetScene().size();
	size_t culledObjects = 0;
	size_t intersectedObjects = 0;
	size_t renderedObjects = 0;

	for (auto it = g.SceneBegin(); it != g.SceneEnd(); ++it)
	{
		const GameObject& gameObject = *(*it);
		if (!gameObject.HasMesh() || !gameObject.IsVisible())
		{
			continue;
		}

		// �������� �ʿ��� ���� ������Ʈ�� �ֿ� ���۷����� ���
		const Mesh& mesh = g.GetMesh(gameObject.GetMeshKey());
		const TransformComponent& transform = gameObject.GetTransform();

		// ���� ��� ���
		Matrix4x4 finalMatrix = pvMatrix * transform.GetWorldMatrix();
		LinearColor finalColor = gameObject.GetColor();

		// ���� ��ȯ ��ķκ��� ����� �����İ� ����ü ����
		Matrix4x4 finalTransposedMatrix = finalMatrix.Transpose();
		std::array<Plane, 6> frustumPlanesFromMatrix = {
			Plane(-(finalTransposedMatrix[3] - finalTransposedMatrix[1])), // up
			Plane(-(finalTransposedMatrix[3] + finalTransposedMatrix[1])), // bottom
			Plane(-(finalTransposedMatrix[3] - finalTransposedMatrix[0])), // right
			Plane(-(finalTransposedMatrix[3] + finalTransposedMatrix[0])), // left 
			Plane(-(finalTransposedMatrix[3] - finalTransposedMatrix[2])),  // far
			Plane(-(finalTransposedMatrix[3] + finalTransposedMatrix[2])), // near
		};
		Frustum frustumFromMatrix(frustumPlanesFromMatrix);

		// �ٿ�� ������ ����� ����ü �ø��� ����
		Box boxBound = mesh.GetBoxBound();
		auto checkResult = frustumFromMatrix.CheckBound(boxBound);
		if (checkResult == BoundCheckResult::Outside)
		{
			culledObjects++;
			continue;
		}
		else if (checkResult == BoundCheckResult::Intersect)
		{
			// ��ģ ���� ������Ʈ�� ��迡 ����
			intersectedObjects++;
		}

		// �޽� �׸���
		DrawMesh3D(mesh, finalMatrix, gameObject.GetColor());

		// �׸� ��ü�� ��迡 ����
		renderedObjects++;
	}
}

// �޽ø� �׸��� �Լ�
void SoftRenderer::DrawMesh3D(const Mesh& InMesh, const Matrix4x4& InMatrix, const LinearColor& InColor)
{
	size_t vertexCount = InMesh.GetVertices().size();
	size_t indexCount = InMesh.GetIndices().size();
	size_t triangleCount = indexCount / 3;

	// �������� ����� ���� ���ۿ� �ε��� ���۷� ��ȯ
	std::vector<Vertex3D> vertices(vertexCount);
	std::vector<size_t> indice(InMesh.GetIndices());
	for (size_t vi = 0; vi < vertexCount; ++vi)
	{
		vertices[vi].Position = Vector4(InMesh.GetVertices()[vi]);

		if (InMesh.IsSkinnedMesh())
		{
			Vector3 totalDeltaPosition;
			Weight w = InMesh.GetWeights()[vi];
			for (size_t wi = 0; wi < InMesh.GetConnectedBones()[vi]; ++wi)
			{
				std::string boneName = w.Bones[wi];
				if (InMesh.HasBone(boneName))
				{
					const Transform& boneTransform = InMesh.GetBone(boneName).GetTransform();
					Vector3 deltaPosition = boneTransform.GetPosition() - InMesh.GetBindPose(boneName).GetPosition();
					totalDeltaPosition += deltaPosition * w.Values[wi];
				}
			}

			vertices[vi].Position += Vector4(totalDeltaPosition, false);
		}

		if (InMesh.HasColor())
		{
			vertices[vi].Color = InMesh.GetColors()[vi];
		}

		if (InMesh.HasUV())
		{
			vertices[vi].UV = InMesh.GetUVs()[vi];
		}
	}

	// ���� ��ȯ ����
	VertexShader3D(vertices, InMatrix);

	// �ﰢ�� ���� �׸���
	for (int ti = 0; ti < triangleCount; ++ti)
	{
		int bi0 = ti * 3, bi1 = ti * 3 + 1, bi2 = ti * 3 + 2;
		std::vector<Vertex3D> tvs = { vertices[indice[bi0]] , vertices[indice[bi1]] , vertices[indice[bi2]] };

		// ������ǥ�迡�� Ŭ������ ���� ����
		std::vector<PerspectiveTest> testPlanes = {
			{ TestFuncW0, EdgeFuncW0 },
			{ TestFuncNY, EdgeFuncNY },
			{ TestFuncPY, EdgeFuncPY },
			{ TestFuncNX, EdgeFuncNX },
			{ TestFuncPX, EdgeFuncPX },
			{ TestFuncFar, EdgeFuncFar },
			{ TestFuncNear, EdgeFuncNear }
		};

		// ������ǥ�迡�� Ŭ���� ����
		for (auto& p : testPlanes)
		{
			p.ClipTriangles(tvs);
		}

		size_t triangles = tvs.size() / 3;
		for (size_t ti = 0; ti < triangles; ++ti)
		{
			size_t si = ti * 3;
			std::vector<Vertex3D> sub(tvs.begin() + si, tvs.begin() + si + 3);
			DrawTriangle3D(sub, InColor, FillMode::Color);
		}
	}
}

// �ﰢ���� �׸��� �Լ�
void SoftRenderer::DrawTriangle3D(std::vector<Vertex3D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
	auto& r = GetRenderer();
	const GameEngine& g = Get3DGameEngine();
	const CameraObject& mainCamera = g.GetMainCamera();

	// ī�޶��� ������ ����� ��
	float n = mainCamera.GetNearZ();
	float f = mainCamera.GetFarZ();

	// Ŭ�� ��ǥ�� NDC ��ǥ�� ����
	for (auto& v : InVertices)
	{
		// ���� ������ ���, �ణ �������ش�.
		if (v.Position.W == 0.f) v.Position.W = SMALL_NUMBER;

		float invW = 1.f / v.Position.W;
		v.Position.X *= invW;
		v.Position.Y *= invW;
		v.Position.Z *= invW;
	}

	// �����̽� �ø�
	Vector3 edge1 = (InVertices[1].Position - InVertices[0].Position).ToVector3();
	Vector3 edge2 = (InVertices[2].Position - InVertices[0].Position).ToVector3();
	Vector3 faceNormal = -edge1.Cross(edge2);
	Vector3 viewDirection = Vector3::UnitZ;
	if (faceNormal.Dot(viewDirection) >= 0.f)
	{
		return;
	}

	// NDC ��ǥ�� ȭ�� ��ǥ�� �ø���
	for (auto& v : InVertices)
	{
		v.Position.X *= _ScreenSize.X * 0.5f;
		v.Position.Y *= _ScreenSize.Y * 0.5f;
	}

	if (IsWireframeDrawing())
	{
		LinearColor finalColor = _WireframeColor;
		if (InColor != LinearColor::Error)
		{
			finalColor = InColor;
		}

		r.DrawLine(InVertices[0].Position, InVertices[1].Position, finalColor);
		r.DrawLine(InVertices[0].Position, InVertices[2].Position, finalColor);
		r.DrawLine(InVertices[1].Position, InVertices[2].Position, finalColor);
	}
	else
	{
		const Texture& mainTexture = g.GetTexture(GameEngine::BaseTexture);

		// �ﰢ�� ĥ�ϱ�
		// �ﰢ���� ���� ����
		Vector2 minPos(Math::Min3(InVertices[0].Position.X, InVertices[1].Position.X, InVertices[2].Position.X), Math::Min3(InVertices[0].Position.Y, InVertices[1].Position.Y, InVertices[2].Position.Y));
		Vector2 maxPos(Math::Max3(InVertices[0].Position.X, InVertices[1].Position.X, InVertices[2].Position.X), Math::Max3(InVertices[0].Position.Y, InVertices[1].Position.Y, InVertices[2].Position.Y));

		// �����߽���ǥ�� ���� ���� ���ͷ� ��ȯ
		Vector2 u = InVertices[1].Position.ToVector2() - InVertices[0].Position.ToVector2();
		Vector2 v = InVertices[2].Position.ToVector2() - InVertices[0].Position.ToVector2();

		// ���� �и� �� ( uu * vv - uv * uv )
		float udotv = u.Dot(v);
		float vdotv = v.Dot(v);
		float udotu = u.Dot(u);
		float denominator = udotv * udotv - vdotv * udotu;

		// ��ȭ �ﰢ�� ����.
		if (denominator == 0.f)
		{
			return;
		}

		float invDenominator = 1.f / denominator;

		ScreenPoint lowerLeftPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, minPos);
		ScreenPoint upperRightPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, maxPos);

		// �� ���� ȭ�� ���� ����� ��� Ŭ���� ó��
		lowerLeftPoint.X = Math::Max(0, lowerLeftPoint.X);
		lowerLeftPoint.Y = Math::Min(_ScreenSize.Y, lowerLeftPoint.Y);
		upperRightPoint.X = Math::Min(_ScreenSize.X, upperRightPoint.X);
		upperRightPoint.Y = Math::Max(0, upperRightPoint.Y);

		// �� �������� ������ �� ������ z��
		float invZ0 = 1.f / InVertices[0].Position.W;
		float invZ1 = 1.f / InVertices[1].Position.W;
		float invZ2 = 1.f / InVertices[2].Position.W;

		// �ﰢ�� ���� �� ��� ���� �����ϰ� ��ĥ
		for (int x = lowerLeftPoint.X; x <= upperRightPoint.X; ++x)
		{
			for (int y = upperRightPoint.Y; y <= lowerLeftPoint.Y; ++y)
			{
				ScreenPoint fragment = ScreenPoint(x, y);
				Vector2 pointToTest = fragment.ToCartesianCoordinate(_ScreenSize);
				Vector2 w = pointToTest - InVertices[0].Position.ToVector2();
				float wdotu = w.Dot(u);
				float wdotv = w.Dot(v);

				float s = (wdotv * udotv - wdotu * vdotv) * invDenominator;
				float t = (wdotu * udotv - wdotv * udotu) * invDenominator;
				float oneMinusST = 1.f - s - t;

				// ���������� ����� ���� �и�
				float z = invZ0 * oneMinusST + invZ1 * s + invZ2 * t;
				float invZ = 1.f / z;

				if (((s >= 0.f) && (s <= 1.f)) && ((t >= 0.f) && (t <= 1.f)) && ((oneMinusST >= 0.f) && (oneMinusST <= 1.f)))
				{
					// ���� �׽���
					float newDepth = InVertices[0].Position.Z * oneMinusST + InVertices[1].Position.Z * s + InVertices[2].Position.Z * t;
					float prevDepth = r.GetDepthBufferValue(fragment);
					if (newDepth < prevDepth)
					{
						// �ȼ��� ó���ϱ� �� ���� ���� ���ۿ� ����
						r.SetDepthBufferValue(fragment, newDepth);
					}
					else
					{
						// �̹� �տ� ���� �׷��������Ƿ� �ȼ��׸���� ����
						continue;
					}

					if (IsDepthBufferDrawing())
					{
						// ī�޶�κ����� �Ÿ��� ���� �����ϰ� �����ϴ� ��� ������ ��ȯ
						float grayScale = (invZ - n) / (f - n);

						// ���� ���� �׸���
						r.DrawPoint(fragment, LinearColor::White * grayScale);
					}
					else
					{
						// ���� ���������� UV ��ǥ
						Vector2 targetUV = (InVertices[0].UV * oneMinusST * invZ0 + InVertices[1].UV * s * invZ1 + InVertices[2].UV * t * invZ2) * invZ;
						r.DrawPoint(fragment, FragmentShader3D(mainTexture.GetSample(targetUV), InColor));
					}
				}
			}
		}
	}
}
