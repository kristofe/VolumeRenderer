#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <map>
#include "vmath.hpp"
#include "Texture.h"
#include "Renderer.h"
#include "RendererObjectFactory.h"
#include "Platform.h"

using namespace vmath;

struct CombinerCommands
{
	std::vector< std::pair<unsigned int, unsigned int> > mCommands;
};



class Material{
	friend class Renderer;
	friend class RendererObjectFactory;
public:
	
	Material(std::string name):mName(name),mTextureCount(0),mIsFont(false)
	{
		mTextureCount = 0;
		memset(mTextureIDs, 0, sizeof(GameID)*MAX_TEXTURES_PER_MATERIAL);
		mDiffuse.Set(1,1,1,1);
		mSpecular.Set(0,0,0,0);
		mAmbient.Set(0, 0, 0, 0);
		mEmissive.Set(0,0,0,0);
		mZWrite = true;
		mZTest = true;
		mBlend = true;
		mBlendFuncA = (int)GL_SRC_ALPHA;
		mBlendFuncB = (int)GL_ONE_MINUS_SRC_ALPHA;
		mUseRenderTarget = false;
		mUseAlphaTest = false;
		mAlphaFunc = (int)GL_GREATER;
		mAlphaFuncTestValue = 0.5f;
		mMultiTexturingEnabled = false;
		mUseColorMask = false;
		
	};
	~Material(){};
private:
	//Remove Copy Constructor and assignment operator
	Material		&operator =  (const Material &m);
	Material(const Material &m);
	
public:
	Color					mDiffuse;
	Color					mSpecular;
	Color					mAmbient;
	Color					mEmissive;
	Color					mColorMask;

	bool					mUseColorMask;
	bool					mUseRenderTarget;
	std::string				mRenderTargetTag;
	GameID					mRenderTargetID;
	
	//GameID				mMainTexture;
	//unsigned int			mMainTextureHWID;
	GameID					mTextureIDs[MAX_TEXTURES_PER_MATERIAL];
	unsigned int			mMainTextureHWIDs[MAX_TEXTURES_PER_MATERIAL];
	int						mTextureCount;

	bool					mIsFont;
	GameID					mGameID;
	std::string				mName;
	bool					mZWrite;
	bool					mZTest;
	bool					mBlend;
	int						mBlendFuncA;
	int						mBlendFuncB;
	bool					mUseAlphaTest;
	int						mAlphaFunc;
	float					mAlphaFuncTestValue;

	Matrix4				mTextureMatrices[MAX_TEXTURES_PER_MATERIAL];
	
	bool					mMultiTexturingEnabled;
	CombinerCommands		mCombinerCommands[MAX_TEXTURES_PER_MATERIAL];
	
	
	
	
public:
	//void AddMesh(Mesh* m);
	//void RemoveMesh(Mesh* m);
	void AddTexture(GameID textureID);
	void RemoveTexture(GameID textureID);
	void SetMainTexture(GameID textureID);
	void ClearTextures();
	GameID GetID(){return mGameID;};
	void Configure(PersistentData& storageData);
	void ConfigureCombiners(PersistentData& storageData);
	void Serialize(PersistentData& storageData);
	void DeSerialize(PersistentData& storageData);

	void				RotateTextureMatrix(Quat& rot, int idx = 0);
	void				RotateTextureMatrix(Vector3& axis, float angle, int idx = 0);
	void				RotateTextureMatrix(float x_angle, float y_angle, float z_angle, int idx = 0);
	void				RotateTextureMatrixX(float angle, int idx = 0);
	void				RotateTextureMatrixY(float angle, int idx = 0);
	void				RotateTextureMatrixZ(float angle, int idx = 0);
	void				TranslateTextureMatrix(const Vector3& p, int idx = 0);	
	void				ScaleTextureMatrix(const Vector3& scale, int idx = 0);
	void				ResetTextureMatrix(int idx = 0) { mTextureMatrices[idx] = Matrix4::identity(); /*mUseTextureMatrix = false;*/};
	//void				SetUseTextureMatrix(bool v) { mUseTextureMatrix = v;};

	
	virtual std::string GetTypeString()
	{
		return "Material";
	}
	
	virtual std::string DebugOutput()
	{
		char buf[256];
		sprintf(buf, "%d\tMaterial\tName:%s",mGameID,mName.c_str());
		std::string s = buf;
		return s;
	}
	
};


#endif //_Geometry_H