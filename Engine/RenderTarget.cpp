#include "RenderTarget.h"
#include "RendererObjectFactory.h"
#include "Texture.h"
#include "Game.h"
#include "Platform.h"
#include "Globals.h"
#include "Scene.h"

////////////////////////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(PersistentData& storageData):SceneObject(storageData)
{
	
	mWidth = storageData.GetPropertyU("width");
	mHeight = storageData.GetPropertyU("height");
	mOffsetX = storageData.GetPropertyU("offsetX");
	mOffsetY = storageData.GetPropertyU("offsetY");
	mHasAlpha = storageData.GetPropertyB("hasAlpha");
	//mRenderEverything = storageData.GetPropertyB("RenderEverything");
	//mCameraTag = storageData.GetProperty("Camera");
	/*if(!mRenderEverything)
	{
		std::list<PersistentData*> renderObjects = storageData.GetNodeFromPath("RenderObjects")->GetChildren();
		std::list<PersistentData*>::iterator it = renderObjects.begin();
		while(it != renderObjects.end())
		{
			mRenderObjectTags.push_back((*it)->GetProperty("Tag"));	
			mExclusiveToRenderTarget.push_back((*it)->GetPropertyB("ExclusiveToTarget"));
			++it;
		}
		
	}*/
	char name[512];
	sprintf(name,"RenderTarget_Tag_%s_%d",mTag.c_str(),mID);
	mTextureID = RendererObjectFactory::CreateTexture(name,name);
	
	
	mTargetTexture = &(RendererObjectFactory::GetTexture(mTextureID));
	mTargetTexture->mWidth = mWidth;
	mTargetTexture->mHeight = mHeight;
	mTargetTexture->mHasAlpha = mHasAlpha;
	mTargetTexture->mDataLength = mWidth*mHeight*4;
	mTargetTexture->mNumberOfColors = 4;
	mTargetTexture->mBitsPerPixel = 32;
	mTargetTexture->mPixelData = (unsigned int*)calloc(mTargetTexture->mDataLength, sizeof(unsigned int));
	
	mGame.GetRenderer().Retain(mID,mTextureID);
	Game::GetInstance().GetRenderer().AddRenderTarget(this);
	/*if(mHasAlpha)
	{
		mTargetTexture->mPixelFormat = PIXELFORMAT_RGBA;
	}
	else
	{
		mTargetTexture->mPixelFormat = PIXELFORMAT_RGB;
	}*/
	
}
////////////////////////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget()
{
	mGame.GetRenderer().Release(mID,mTextureID);
}
////////////////////////////////////////////////////////////////////////////////
void RenderTarget::LevelLoaded()
{
	/*for(unsigned int i = 0; i < mRenderObjectTags.size(); ++i)
	{
		
		RenderObject* ro = (RenderObject*)Game::GetInstance().GetTaggedObject(mRenderObjectTags[i]);
		ro->SetExclusiveToRenderTarget(mExclusiveToRenderTarget[i]);
		if(ro->IsTransparent())
			mTranparentRenderObjects.push_back(ro);
		else
			mOpaqueRenderObjects.push_back(ro);	
		
	}*/

}
////////////////////////////////////////////////////////////////////////////////
void RenderTarget::Serialize(PersistentData& storageData)
{
	GameObject::Serialize(storageData);
	storageData.SetName("RenderTarget");
	
}
////////////////////////////////////////////////////////////////////////////////
void RenderTarget::DeSerialize(PersistentData& storageData)
{
	GameObject::DeSerialize(storageData);
	
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////