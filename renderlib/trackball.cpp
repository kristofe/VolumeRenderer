#include "include/trackball.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/norm.hpp"

using namespace renderlib;
using namespace glm;


quat Trackball::RotationBetweenVectors(vec3 start, vec3 dest) const
{
    start = normalize(start);
    dest = normalize(dest);
 
    float cosTheta = dot(start, dest);
    vec3 rotationAxis;
 
    if (cosTheta < -1 + 0.001f){
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
            rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);
 
        rotationAxis = normalize(rotationAxis);
        return glm::angleAxis(180.0f, rotationAxis);
    }
 
    rotationAxis = cross(start, dest);
 
    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;
 
    return quat(
        s * 0.5f, 
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs
    );
 
}

Trackball::Trackball(float width, float height, float radius)
{
    m_currentTime = 0;
    m_inertia.Active = false;
    m_voyageHome.Active = false;
    m_active = false;
    m_quat = quat();
    m_radius = radius;
    m_startPos = m_currentPos = m_previousPos = vec3(0);
    m_width = width;
    m_height = height;
    m_startZoom = m_zoom = 0;
}

void Trackball::MouseDown(int x, int y)
{
    m_radiansPerSecond = 0;
    m_distancePerSecond = 0;
    m_previousPos = m_currentPos = m_startPos = MapToSphere(x, y);
    m_active = true;
    m_startZoom = m_zoom;
    m_startY = y;
}


void Trackball::MouseUp(int x, int y)
{
    if (m_active) {
        float deltaDistance = (y - m_startY) * 0.01f;
        m_zoom = m_startZoom + deltaDistance;
        m_startZoom = m_zoom;
        m_startY = y;
        m_active = false;
    }

    quat q =RotationBetweenVectors(m_startPos, m_currentPos);
    //m_quat = rotate(q, m_quat);
    m_quat = q * m_quat;

    if (m_radiansPerSecond > 0 || m_distancePerSecond != 0) {
        m_inertia.Active = true;
        m_inertia.RadiansPerSecond = m_radiansPerSecond;
        m_inertia.DistancePerSecond = m_distancePerSecond;
        m_inertia.Axis = m_axis;
    }
}

void Trackball::MouseMove(int x, int y)
{
    m_currentPos = MapToSphere(x, y);

    float radians = acos(dot(m_previousPos, m_currentPos));
    unsigned int microseconds = m_currentTime - m_previousTime;

    if (radians > 0.01f && microseconds > 0) {
        m_radiansPerSecond = 1000000.0f * radians / microseconds;
        m_axis = normalize(cross(m_previousPos, m_currentPos));
    } else {
        m_radiansPerSecond = 0;
    }

    if (m_active) {
        float deltaDistance = (y - m_startY) * 0.01f;
        if (std::abs(deltaDistance) > 0.03f && microseconds > 0) {
            m_distancePerSecond = 1000000.0f * deltaDistance / microseconds;
        } else {
            m_distancePerSecond = 0;
        }

        m_zoom = m_startZoom + deltaDistance;
    }

    m_startZoom = m_zoom;
    m_startY = y;

    m_previousPos = m_currentPos;
    m_previousTime = m_currentTime;
}

mat3 Trackball::GetRotation() const
{
    if (!m_active)
        return toMat3(m_quat);

	quat q = RotationBetweenVectors(m_startPos, m_currentPos);
    //Quat q = Quat::rotation(m_startPos, m_currentPos);
    return toMat3(q * m_quat);
}

vec3 Trackball::MapToSphere(int x, int y)
{
    x = int(m_width) - x;
    const float SafeRadius = m_radius * 0.99f;
    float fx = x - m_width / 2.0f;
    float fy = 0; // y - m_height / 2.0f;

    float lenSqr = fx*fx+fy*fy;

    if (lenSqr > SafeRadius*SafeRadius) {
        float theta = atan2(fy, fx);
        fx = SafeRadius * cos(theta);
        fy = SafeRadius * sin(theta);
    }

    lenSqr = fx*fx+fy*fy;
    float z = sqrt(m_radius*m_radius - lenSqr);
    return glm::vec3(fx, fy, z) / m_radius;
}

void Trackball::Update(unsigned int microseconds)
{
    m_currentTime += microseconds;

    if (m_voyageHome.Active) {
        m_voyageHome.microseconds += microseconds;
        float t = m_voyageHome.microseconds / 200000.0f;

        if (t > 1) {
            //m_quat = Quat::identity();
            m_quat = quat();
            m_startZoom = m_zoom = 0;
            m_voyageHome.Active = false;
            return;
        }

        //m_quat = slerp(t, m_voyageHome.DepartureQuat, Quat::identity());
		m_quat = glm::slerp( m_voyageHome.DepartureQuat, quat(),t);
        m_startZoom = m_zoom = m_voyageHome.DepartureZoom * (1-t);
        m_inertia.Active = false;
    }

    if (m_inertia.Active) {
        m_inertia.RadiansPerSecond -= 0.00001f * microseconds;

        if (m_inertia.RadiansPerSecond < 0) {
            m_radiansPerSecond = 0;
        } else {
            quat q = rotate(q,m_inertia.RadiansPerSecond * microseconds * 0.000001f, m_inertia.Axis);
            m_quat = q * m_quat;
        }

        m_inertia.DistancePerSecond *= 0.75f;
        /*
        if (m_inertia.DistancePerSecond > 0) {
            m_inertia.DistancePerSecond -= 1.0f;
            if (m_inertia.DistancePerSecond < 0)
                m_inertia.DistancePerSecond = 0;
        }

        if (m_inertia.DistancePerSecond < 0) {
            m_inertia.DistancePerSecond += 1.0f;
            if (m_inertia.DistancePerSecond > 0)
                m_inertia.DistancePerSecond = 0;
        }
        */
        if (std::abs(m_inertia.DistancePerSecond) < 0.0001) {
            m_distancePerSecond = 0;
        } else {
            m_zoom += m_distancePerSecond * 0.001f;
        }

        if (std::abs(m_inertia.DistancePerSecond) < 0.0001 && m_inertia.RadiansPerSecond < 0)
            m_inertia.Active = false;
    }
}

void Trackball::ReturnHome()
{
    m_voyageHome.Active = true;
    m_voyageHome.DepartureQuat = m_quat;
    m_voyageHome.DepartureZoom = m_zoom;
    m_voyageHome.microseconds = 0;
}

float Trackball::GetZoom() const
{
    return m_zoom;
}


