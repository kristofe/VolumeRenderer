/*
 *  SceneObject.cpp
 *  XPlatformGL
 *
 *  Created by Kristofer Schlachter on 1/11/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "SceneObject.h"
#include "Game.h"

////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject():mParent(NULL),mRoot(NULL),mPhysicsProxy(NULL){
	mLocalPosition.Set(0,0,0);
	mLocalRotation.Set(0,0,0);
	mLocalScale.Set(1,1,1);
	
}
////////////////////////////////////////////////////////////////////////////////
SceneObject::SceneObject(PersistentData& storageData):GameObject(storageData),mParent(NULL),mRoot(NULL),mPhysicsProxy(NULL){
	mLocalPosition.Set(0,0,0);
	mLocalRotation.Set(0,0,0);
	mLocalScale.Set(1,1,1);
	Vector3 pos, scale, rot;

	
	if(storageData.GetNodeFromPath("Position") != NULL)
	{
		pos = storageData.GetVector3Child("Position");
		Translate(pos);
	}
	if(storageData.GetNodeFromPath("Rotation") != NULL)
	{
		rot = storageData.GetVector3Child("Rotation");
		Rotate(rot.x,rot.y,rot.z);
	}
	if(storageData.GetNodeFromPath("Scale") != NULL)
	{
		scale = storageData.GetVector3Child("Scale");
		Scale(scale);
	}
	/*<Physics>
	 <Collider type="BOX">
	 <Extents x="10" y="10" z="10"/>
	 </Collider>
	 <RigidBody mass="1.0"/>
	 </Physics>*/
	
	//ColliderID cID = mGame.GetPhysicsManager().ConstructBoxCollider(Vector3(25.0f,35.0f,5.0f));
	//mPhysicsProxy = new PhysicsProxy((SceneObject&)*this,cID,true, 1.0f);
	
//	PersistentData* phys = storageData.GetNodeFromPath("Physics");
//	if(phys != NULL)
//	{
//		ColliderID cID = -1;
//		float mass = 0.0f;
//		bool makeRigidBody = false;
//		bool isTrigger = false;
//		PersistentData* collider = phys->GetNodeFromPath("Collider");
//		if(collider!= NULL)
//		{
//			cID = mGame.GetPhysicsManager().ConstructCollider(*collider);
//			isTrigger = collider->GetPropertyB("isTrigger");
//		}
//		if(phys->GetNodeFromPath("RigidBody") != NULL)
//		{
//			makeRigidBody = true;
//			mass = phys->GetNodeFromPath("RigidBody")->GetPropertyB("mass");
//		}
//		if(cID >= 0)
//		{
//			mPhysicsProxy = new PhysicsProxy((SceneObject*)this,cID,makeRigidBody,isTrigger,mass);
//		}
//	}
}

////////////////////////////////////////////////////////////////////////////////
SceneObject::~SceneObject(){
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::AddedToDeletionList()
{
	std::vector<SceneObject*>::iterator it = mChildren.begin();
	while(it != mChildren.end()){
		mGame.DeleteGameObject((*it)->GetID());
		++it;
	}
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::Serialize(PersistentData& storageData)
{
//	GameObject::Serialize(storageData);
//	storageData.SetName("SceneObject");
//	
//	storageData.AddVector3Child(mLocalPosition,"Position");
//	storageData.AddQuatChild(mLocalRotation,"Rotation");
//	storageData.AddVector3Child(mLocalScale,"Scale");
//	
//	char buff[128];
//	unsigned int objID =  (unsigned int)(mRoot != NULL?mRoot->GetID():0);
//	
//	sprintf(buff, "%d",objID);
//	storageData.SetProperty("Root",buff);
//	
//	objID = (unsigned int)(mParent != NULL?mParent->GetID():0);
//	sprintf(buff, "%d",objID);
//	storageData.SetProperty("Parent",buff);
//	PersistentData* children = new PersistentData("Children");
//	storageData.AddChild(children);
//	
//	for(int i = 0; i < (int)mChildren.size(); ++i)
//	{
//		PersistentData* c = new PersistentData("Child");
//		objID = static_cast<unsigned int>(mChildren[i]->GetID());
//		sprintf(buff, "%d",objID);
//		std::string s = buff;
//		c->SetTextProperty(s);
//		children->AddChild(c);
//		
//	}
//	
//	storageData.AddMatrix4Child(mLocalTransform,"Transform");
//	
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::DeSerialize(PersistentData& storageData)
{
	GameObject::DeSerialize(storageData);
	mLocalPosition = storageData.GetVector3Child("Position");
	mLocalRotation = storageData.GetQuatChild("Rotation");
	mLocalScale = storageData.GetVector3Child("Scale");
	

	mID = static_cast<GameID>(storageData.GetPropertyU("Root"));
	mOwner = static_cast<GameID>(storageData.GetPropertyU("Owner"));
	std::list<PersistentData*> children;
	storageData.GetNodesFromPath("Children",children);
	std::list<PersistentData*>::iterator it = children.begin();
	while(it != children.end())
	{
		PersistentData* c = (*it);
		GameID id = static_cast<GameID> (c->GetTextAsUInt());
		//mGame Better Not Be NULL At This Point!!!
		SceneObject* so = dynamic_cast<SceneObject*>(mGame.GetGameObject(id));
		mChildren.push_back(so);
		++it;
	}
	
	Matrix4 mtx = storageData.GetMatrix4Child("Transform");
	
	if(!mtx.IsIdentity())
	{
		mLocalTransform = mtx;
		UpdateFromNewLocalTransform_NoMutex();
	}
	else
	{
		Translate(mLocalPosition);
		Rotate(mLocalRotation);
		Scale(mLocalScale);
		
		
	}
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::SetLocalTransform(const Matrix4& m){
	mDirty = true;
	//UpdateOrientationAndPositionData();
	mLocalTransform = m;
	
	//Now update all of the other transforms
	UpdateFromNewLocalTransform_NoMutex();
	
	
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::UpdateChildren(){
	std::vector<SceneObject*>::iterator it = mChildren.begin();
	while(it != mChildren.end()){
		(*it)->UpdateFromParentWorldTransform_NoMutex(mWorldTransform);
		(*it)->UpdateDirectionAndPositionVectors_NoMutex();
		(*it)->UpdateChildren();
		++it;
	}

	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::UpdateFromParentWorldTransform_NoMutex(const Matrix4& parentWorldTransform){
	mWorldTransform = parentWorldTransform * mLocalTransform;
	mWorldInverseTransform = affineInverse(mWorldTransform);
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::UpdateDirectionAndPositionVectors_NoMutex(){
	mLocalTransform.GetColumns(mLocalRight,mLocalUp, mLocalForward,mLocalPosition);
	mWorldTransform.GetColumns(mRight,mUp, mForward,mPosition);
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::UpdateFromNewLocalTransform_NoMutex(){
	mLocalInverseTransform = affineInverse(mLocalTransform);
	if(mParent != NULL){
		Matrix4 parentTransform;
		mParent->GetWorldTransform(parentTransform);
		UpdateFromParentWorldTransform_NoMutex(parentTransform);
	}else{
		mWorldTransform = mLocalTransform;
		mWorldInverseTransform = affineInverse(mWorldTransform);
	}
	
	UpdateDirectionAndPositionVectors_NoMutex();
	UpdateChildren();
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::Translate(const Vector3& p){
	mDirty = true;
	Matrix4 m = Matrix4::translation(p);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::SetPosition(const Vector3& p){
	mDirty = true;

	mLocalTransform.SetPosition(p);
	UpdateFromNewLocalTransform_NoMutex();
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::SetWorldPosition(const Vector3& worldPos){
	mDirty = true;

	if(mParent == NULL)
	{
		mLocalTransform.SetPosition(worldPos);
	}
	else
	{
		//put the new position into the parent's coordinate frame
		Matrix4 mtx;		
		mParent->GetWorldInverseTransform(mtx);
		
		Vector3 pos = mtx.TransformPoint(worldPos);
		mLocalTransform.SetPosition(pos);
		
	}
	UpdateFromNewLocalTransform_NoMutex();
	
}


////////////////////////////////////////////////////////////////////////////////
void SceneObject::SetRoot(SceneObject* r){
	mDirty = true;
	mRoot = r;
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::SetParent(SceneObject* p){
	//Physics system uses world transfroms so this cannot be a child of anything if it is part of the physics system
	if(mPhysicsProxy != NULL && p != NULL)
	{
		printf("Physics system uses world transfroms so this SceneObject (ID=%d) cannot be a child of anything if it is part of the physics system", mID);
		return;
	}
	mDirty = true;
	mParent = p;
	Matrix4 parentWorldTransform;
	if(p != NULL){
		mRoot = p->GetRoot();
		mParent->GetWorldTransform(parentWorldTransform);
	}else{
		mRoot = NULL;
	}
	UpdateFromParentWorldTransform_NoMutex(parentWorldTransform);
	UpdateDirectionAndPositionVectors_NoMutex();
	
	//if(p != NULL)
	//	p->UpdateChildren();
	
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::AddChild(SceneObject* t){
	mDirty = true;
	mChildren.push_back(t);
	
	t->SetParent(this);
		
}

////////////////////////////////////////////////////////////////////////////////
bool SceneObject::HasChild(SceneObject* t){
	for(unsigned int i = 0; i < mChildren.size(); ++i){
		SceneObject* obj = mChildren[i];
		if(obj == t){
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::RemoveChild(SceneObject* t){
	
	
	bool found = false;
	std::vector<SceneObject*>::iterator it = mChildren.begin();
	while(it != mChildren.end()){
		if(*it == t){
			found = true;
			break;
		}
		++it;
	}
	if(found && it != mChildren.end()){
		mDirty = true;
		(*it)->SetParent(NULL);
		mChildren.erase(it);
		
	}
	
}


////////////////////////////////////////////////////////////////////////////////
unsigned int SceneObject::GetNumChildren(){
	return (unsigned int)mChildren.size();
}

////////////////////////////////////////////////////////////////////////////////
bool SceneObject::IsChildOf(SceneObject* t){
	//Have to traverse the tree!
	std::vector<SceneObject*>::iterator it = mChildren.begin();
	while(it != mChildren.end()){
		if(*it == t){
			return true;
		}else{
			return (*it)->IsChildOf(t);
		}
		++it;
	}
	return false;
}


 //Need Multiple Of the following
////////////////////////////////////////////////////////////////////////////////
void SceneObject::Rotate(Matrix3& rot){
	mDirty = true;
	Quat r(rot);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(r);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::Rotate(Quat& rot){
	mDirty = true;
	mLocalRotation = rot;
	Matrix4 m = Matrix4::rotation(rot);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}
	
////////////////////////////////////////////////////////////////////////////////
void SceneObject::Rotate(Vector3& axis, float angle){
	mDirty = true;
	Quat r(axis, angle);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(angle, axis);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::Rotate(float x_angle, float y_angle, float z_angle){
	mDirty = true;
	Quat r;
	r.Set(x_angle,y_angle,z_angle);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(r);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::RotateX(float angle){
	mDirty = true;
	Vector3 axis = Vector3::xAxis();
	Quat r(axis, angle);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(r);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::RotateY(float angle){
	mDirty = true;
	Vector3 axis = Vector3::yAxis();
	Quat r(axis, angle);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(r);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::RotateZ(float angle){
	mDirty = true;
	Vector3 axis = Vector3::zAxis();
	Quat r(axis, angle);
	mLocalRotation = r;
	Matrix4 m = Matrix4::rotation(r);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::Scale(const Vector3& scale){
	mDirty = true;
	mLocalScale = scale;
	Matrix4 m = Matrix4::scale(scale);
	mLocalTransform *= m;
	UpdateFromNewLocalTransform_NoMutex();
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::ResetLocalTransform(){
	mDirty = true;
	mLocalScale.x = mLocalScale.y = mLocalScale.z = 1.0f;
	mLocalTransform = Matrix4::identity();
	UpdateFromNewLocalTransform_NoMutex();
	
}

////////////////////////////////////////////////////////////////////////////////
void SceneObject::LookAt(Vector3& position, Vector3& targetPoint, Vector3& up){
	mDirty = true;
	Matrix4 m;
	
	Vector3 dir3 = targetPoint - position;
	dir3 = normalize(dir3);
	
	Vector3 right3 = cross(dir3,up);
	
	Vector4 upAxis(up);
	Vector4 fwdAxis(dir3);
	Vector4 rightAxis(right3);
	Vector4 pos(position,1);
	
	
	m.SetColumns(rightAxis,upAxis,fwdAxis,pos);

	mLocalTransform = m;
	UpdateFromNewLocalTransform_NoMutex();
	
}
////////////////////////////////////////////////////////////////////////////////
void	SceneObject::OnCollision(PhysicsProxy& other, Vector3& contactPointWorldCoord, Vector3& normal)
{
	//printf("Collision %d-%d, at (%3.2f,%3.2f,%3.f) direction (%3.2f,%3.2f,%3.f)\n",mID, other.mSceneObject.mID, contactPointWorldCoord.x,contactPointWorldCoord.y,contactPointWorldCoord.z, normal.x, normal.y, normal.z);
}
////////////////////////////////////////////////////////////////////////////////
void	SceneObject::OnTriggerEnter(PhysicsProxy& other, Vector3& contactPointWorldCoord, Vector3& normal)
{
	//printf("Trigger Enter %d-%d, at (%3.2f,%3.2f,%3.f) direction (%3.2f,%3.2f,%3.f)\n",mID, other.mSceneObject.mID, contactPointWorldCoord.x,contactPointWorldCoord.y,contactPointWorldCoord.z, normal.x, normal.y, normal.z);
}
////////////////////////////////////////////////////////////////////////////////
void	SceneObject::OnTriggerStay(PhysicsProxy& other, Vector3& contactPointWorldCoord, Vector3& normal)
{
	//printf("TriggerStay ");
	//printf("Collision %d-%d, at (%3.2f,%3.2f,%3.f) direction (%3.2f,%3.2f,%3.f)\n",mID, other.mSceneObject.mID, contactPointWorldCoord.x,contactPointWorldCoord.y,contactPointWorldCoord.z, normal.x, normal.y, normal.z);
}
////////////////////////////////////////////////////////////////////////////////
void	SceneObject::OnTriggerLeave(PhysicsProxy& other)
{
	//printf("\n\nTrigger Left\n");
	//printf("Collision %d-%d, at (%3.2f,%3.2f,%3.f) direction (%3.2f,%3.2f,%3.f)\n",mID, other.mSceneObject.mID, contactPointWorldCoord.x,contactPointWorldCoord.y,contactPointWorldCoord.z, normal.x, normal.y, normal.z);
}
////////////////////////////////////////////////////////////////////////////////
/*void SceneObject::AddCollider(ColliderID colliderID)
{
	if(mPhysicsProxy == NULL)
	{
		mPhysicsProxy = new PhysicsProxy(this, Game::GetInstance().GetPhysicsManager());
	}
	mPhysicsProxy->mCollisionShapeID = colliderID
	mPhysicsProxy->mMass = 0.0f;
	
}
////////////////////////////////////////////////////////////////////////////////
void SceneObject::AddRigidBody(RigidBodyID rbID, ColliderID colliderID, float mass)
{
	if(mPhysicsProxy == NULL)
	{
		mPhysicsProxy = new PhysicsProxy(this, Game::GetInstance().GetPhysicsManager());
	}
	mPhysicsProxy->mRigidBodyID = rbID;
	mPhysicsProxy->mCollisionShapeID = colliderID
	mPhysicsProxy->mMass = mass;
}*/
////////////////////////////////////////////////////////////////////////////////

 