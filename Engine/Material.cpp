#include <string>
#include <iostream>
#include <fstream>
#include "Material.h"
#include "Game.h"
#include "RenderTarget.h"

////////////////////////////////////////////////////////////////////////////////
/* REMOVED THESE BECAUSE IT IS HARD TO TELL WHAT MESHES ARE ACTIVE IF WE TRACK ONLY 
   THROUGH MATERIAL
void Material::AddMesh(Mesh* m)
{
	std::map<RendererMeshID, Mesh*>::iterator it;
	it = mMeshes.find(m->GetGameID());
	if(it != mMeshes.end())
	{
		return;
	}
	mMeshes[m->GetGameID()] = m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RemoveMesh(Mesh* m)
{
	std::map<RendererMeshID, Mesh*>::iterator it;
	it = mMeshes.find(m->GetGameID());
	if(it != mMeshes.end())
	{
		mMeshes.erase(it);
	}

}
*/
////////////////////////////////////////////////////////////////////////////////
void Material::Configure(PersistentData& storageData)
{
	mUseRenderTarget = false;
	mDiffuse = storageData.GetColorChild("Diffuse");
	mSpecular = storageData.GetColorChild("Specular");
	mAmbient = storageData.GetColorChild("Ambient");
	mEmissive = storageData.GetColorChild("Emissive");
	mColorMask = storageData.GetColorChild("ColorMask");
	if(mColorMask != Color::white)
	{
		mUseColorMask = true;
	}
	
	mZWrite = !storageData.GetPropertyB("DisableZWrite");
	mZTest = !storageData.GetPropertyB("DisableZTest");

	PersistentData* texturesParent = storageData.GetNodeFromPath("Textures");
	if(texturesParent != NULL)
	{
		ClearTextures();
		std::list<PersistentData*> textures;
		texturesParent->GetNodesFromPath("Texture",textures);
		std::list<PersistentData*>::iterator txtIt = textures.begin();
		while(txtIt != textures.end())
		{
			PersistentData* txNode = *txtIt;
			//////////////////////////
			
			mRenderTargetTag = txNode->GetProperty("UseRenderTarget");
			if(mRenderTargetTag.empty() == false)
			{
				RenderTarget* rt = (RenderTarget*)Game::GetInstance().GetTaggedObject(mRenderTargetTag);
				if(rt != NULL)
				{
					mUseRenderTarget = true;
					AddTexture(rt->GetTextureID());
				}
				else
				{
					printf("ERROR: Material '%s' wants to use non-existent render target: %s\n",mName.c_str(),mRenderTargetTag.c_str());
				}
			}
			else
			{
				std::string imagename = txNode->GetProperty("ImageName");
				bool delayLoad = txNode->GetPropertyB("DelayLoad");
				GameID texID = RendererObjectFactory::CreateTexture(imagename.c_str(),imagename.c_str(),delayLoad);
				AddTexture(texID);				
			}
			//////////////////////////
			++txtIt;
		}
		
	}
	

	PersistentData* tmParent = storageData.GetNodeFromPath("TextureMatrices");
	if(tmParent != NULL)
	{
		std::list<PersistentData*> textureMatrices;
		tmParent->GetNodesFromPath("TextureMatrix",textureMatrices);
		
		Vector3 pos, scale, rot;
		int idx = 0;
		std::list<PersistentData*>::iterator tmNodeIt = textureMatrices.begin();
		while(tmNodeIt != textureMatrices.end())
		{
			PersistentData* tmNode =*tmNodeIt;
			if(tmNode != NULL){
				//mUseTextureMatrix = true;
				if(tmNode->GetNodeFromPath("Position") != NULL)
				{
					pos = tmNode->GetVector3Child("Position");
					TranslateTextureMatrix(pos,idx);
				}
				if(storageData.GetNodeFromPath("Rotation") != NULL)
				{
					rot = tmNode->GetVector3Child("Rotation");
					RotateTextureMatrix(rot.x,rot.y,rot.z,idx);
				}
				if(tmNode->GetNodeFromPath("Scale") != NULL)
				{
					scale = tmNode->GetVector3Child("Scale");
					ScaleTextureMatrix(scale,idx);
				}
			}
			++idx;
			++tmNodeIt;
		}
		
	}
	
	PersistentData* tc = storageData.GetNodeFromPath("TextureCombiners");
	if(tc != NULL)
	{
		ConfigureCombiners(*tc);
	}
	
	std::string blendFunc;
	Renderer& rend = Game::GetInstance().GetRenderer();
	
	PersistentData* blend = storageData.GetNodeFromPath("Blend");
	mBlend = blend->GetPropertyB("Enabled");

	blendFunc = blend->GetProperty("FuncA");
	mBlendFuncA = rend.GetEnumForString(blendFunc);
	
	blendFunc = blend->GetProperty("FuncB");
	mBlendFuncB = rend.GetEnumForString(blendFunc);

	
	std::string alphaFunc;
	PersistentData* alphaTest = storageData.GetNodeFromPath("AlphaTest");
	if(alphaTest != NULL)
	{
		mUseAlphaTest = alphaTest->GetPropertyB("Enabled");
		mAlphaFuncTestValue = alphaTest->GetPropertyF("value");
		alphaFunc = alphaTest->GetProperty("Func");
		mAlphaFunc = rend.GetEnumForString(alphaFunc);
		
	}
	
	/*
	 //Causes Crash on 3.0 Release but not 3.0 debug
	 PersistentData* shaderInfo = storageData.GetNodeFromPath("Shaders");
	if(shaderInfo != NULL)
	{
		mVertexShaderPath = shaderInfo->GetProperty("vertex");
		mFragmentShaderPath = shaderInfo->GetProperty("fragment");
		SetupShaders();
	}*/
}

////////////////////////////////////////////////////////////////////////////////
void Material::ConfigureCombiners(PersistentData& storageData)
{
	//bool					mMultitexturingEnabled;
	//CombinerCommands		mCombinerCommands[MAX_TEXTURES_PER_MATERIAL];
	std::list<PersistentData*> textureBindings;
	storageData.GetNodesFromPath("TextureBinding",textureBindings);
	Renderer& rend = Game::GetInstance().GetRenderer();
	
	unsigned int textureUnit = 0;
	std::list<PersistentData*>::iterator textureBindingsIt = textureBindings.begin();
	while(textureBindingsIt != textureBindings.end())
	{
		
		PersistentData* tb = *textureBindingsIt;
		if(textureUnit >= MAX_TEXTURES_PER_MATERIAL)
			break;
		
		mMultiTexturingEnabled = true;
		
		CombinerCommands& cc = mCombinerCommands[textureUnit];
		//cc.mTextureUnit = textureUnit;
		//cc.mTextureUnitEnum =  rend.GetEnumForString(tb->GetProperty("Op"));
		
		
		std::list<PersistentData*> textureEnvCmds;
		tb->GetNodesFromPath("TextureEnv",textureEnvCmds);
		std::list<PersistentData*>::iterator txEnvIt = textureEnvCmds.begin();
		while(txEnvIt != textureEnvCmds.end())
		{
			unsigned int op0, op1;
			
			PersistentData* txEnvCmd = *txEnvIt;
			op0 = rend.GetEnumForString(txEnvCmd->GetProperty("Op1"));
			op1 = rend.GetEnumForString(txEnvCmd->GetProperty("Op2"));
			
			if(op0 == 0)
			{
				printf("ERROR!!!! There was an error while setting up TextureCombiners... couldn't translate op1 %s\n", txEnvCmd->GetProperty("Op1").c_str());
			}
			if(op1 == 0)
			{
				printf("ERROR!!!! There was an error while setting up TextureCombiners... couldn't translate op2 %s\n", txEnvCmd->GetProperty("Op2").c_str());
			}
					   
			cc.mCommands.push_back(std::pair<unsigned int, unsigned int>(op0,op1));
			
			++txEnvIt;
		}
		++textureUnit;
		++textureBindingsIt;
	}
	
}
////////////////////////////////////////////////////////////////////////////////
void Material::Serialize(PersistentData& storageData)
{
	storageData.SetName("Material");
	
	storageData.SetProperty("Name",mName.c_str());
	storageData.AddColorChild(mDiffuse,"Diffuse");
	storageData.AddColorChild(mSpecular,"Specular");
	storageData.AddColorChild(mAmbient,"Ambient");
	storageData.AddColorChild(mEmissive,"Emissive");
	//storageData.SetProperty("MainTexture",mMainTexture);
	storageData.SetProperty("TextureCount",mTextureCount);
	storageData.SetProperty("IsFont",mIsFont);
	storageData.SetProperty("GameID",mGameID);

	
	
	char buff[128];
	unsigned int objID;
	//GameID				mTextureIDs[MAX_TEXTURES_PER_MATERIAL];
	PersistentData* textures = new PersistentData("Textures");
	storageData.AddChild(textures);
	
	for(int i = 0; i < MAX_TEXTURES_PER_MATERIAL; ++i)
	{
		PersistentData* c = new PersistentData("Texture");
		objID = static_cast<unsigned int>(mTextureIDs[i]);
		sprintf(buff, "%d",objID);
		std::string s = buff;
		c->SetTextProperty(s);
		textures->AddChild(c);
	}

	
	
}
////////////////////////////////////////////////////////////////////////////////
void Material::DeSerialize(PersistentData& storageData)
{	
	mDiffuse = storageData.GetColorChild("Diffuse");
	mSpecular = storageData.GetColorChild("Specular");
	mAmbient = storageData.GetColorChild("Ambient");
	mEmissive = storageData.GetColorChild("Emissive");

	//mMainTexture = static_cast<GameID> (storageData.GetPropertyU("MainTexture"));
	mTextureCount = storageData.GetPropertyI("TextureCount");
	mIsFont = storageData.GetPropertyB("IsFont");
	mName = storageData.GetProperty("Name");
	mGameID = static_cast<GameID> (storageData.GetPropertyU("GameID"));
	
	std::list<PersistentData*> children;
	
	storageData.GetNodesFromPath("Textures",children);
	std::list<PersistentData*>::iterator it = children.begin();
	int i = 0;
	while(it != children.end() && i < MAX_TEXTURES_PER_MATERIAL)
	{
		PersistentData*c = (*it);
		GameID texID = static_cast<GameID> (c->GetTextAsUInt());
		mTextureIDs[i++] = texID;
		++it;
	}
	
	RendererObjectFactory::RegisterMaterial(mGameID, mName.c_str());
}


////////////////////////////////////////////////////////////////////////////////
void Material::AddTexture(GameID textureID)
{
	if(mTextureCount >= MAX_TEXTURES_PER_MATERIAL - 1)
		return;
	
	mTextureIDs[mTextureCount] = textureID;
	
	Renderer& r = Game::GetInstance().GetRenderer();
	r.Retain(mGameID,textureID);
	
	mTextureCount++;
	

}
////////////////////////////////////////////////////////////////////////////////
void Material::RemoveTexture(GameID textureID)
 {
	 std::vector<GameID> textures;
	 Renderer& r = Game::GetInstance().GetRenderer();
	 
	 for(int i = 0; i < mTextureCount; i++)
	 {
		 GameID id = mTextureIDs[i];
		 if(mTextureIDs[i] == textureID)
		 {
			 r.Release(mGameID,id);
		 }
		 else
		 {
			textures.push_back(id); 
		 }
		 mTextureIDs[i] = 0;
	 }
	 
	 mTextureCount = 0;
	 for(unsigned int j = 0; j < textures.size(); j++)
	 {
		 mTextureIDs[j] = textures[j];
		 mTextureCount++;
	 }
 
 
 }
////////////////////////////////////////////////////////////////////////////////
void Material::SetMainTexture(GameID textureID)
{	
	if(mTextureIDs[0] != 0)
	{
		Renderer& r = Game::GetInstance().GetRenderer();
		r.Release(mGameID, mTextureIDs[0]);
	}
	
	mTextureIDs[0] = textureID;
	if(mTextureCount == 0)
	{
		mTextureCount++;
	}
}
////////////////////////////////////////////////////////////////////////////////
void Material::ClearTextures()
{
	Renderer& r = Game::GetInstance().GetRenderer();
	for(int i = 0; i < mTextureCount; i++)
	{
		r.Release(mGameID,mTextureIDs[i]);
		mTextureIDs[i] = 0;
	}
	
	//memset(mTextureIDs, 0, sizeof(GameID)*MAX_TEXTURES_PER_MATERIAL);
	mTextureCount = 0;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrix(Quat& rot, int idx)
{
	//mUseTextureMatrix = true;
	Matrix4 m = Matrix4::rotation(rot);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrix(Vector3& axis, float angle, int idx)
{
	//mUseTextureMatrix = true;
	Matrix4 m = Matrix4::rotation(angle, axis);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrix(float x_angle, float y_angle, float z_angle, int idx)
{
	//mUseTextureMatrix = true;
	Quat r;
	r.Set(x_angle,y_angle,z_angle);
	Matrix4 m = Matrix4::rotation(r);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrixX(float angle, int idx)
{
	//mUseTextureMatrix = true;
	Vector3 axis = Vector3::xAxis();
	Quat r(axis, angle);
	Matrix4 m = Matrix4::rotation(r);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrixY(float angle, int idx)
{
	//mUseTextureMatrix = true;
	Vector3 axis = Vector3::yAxis();
	Quat r(axis, angle);
	Matrix4 m = Matrix4::rotation(r);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::RotateTextureMatrixZ(float angle, int idx)
{
	Vector3 axis = Vector3::zAxis();
	Quat r(axis, angle);
	Matrix4 m = Matrix4::rotation(r);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::TranslateTextureMatrix(const Vector3& p, int idx)
{
	//mUseTextureMatrix = true;
	Matrix4 m = Matrix4::translation(p);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////
void Material::ScaleTextureMatrix(const Vector3& scale, int idx)
{
	//mUseTextureMatrix = true;
	Matrix4 m = Matrix4::scale(scale);
	mTextureMatrices[idx] *= m;
}
////////////////////////////////////////////////////////////////////////////////