#ifndef RENDERLIB_TRACKBALL_H
#define RENDERLIB_TRACKBALL_H
#include "glutil.h"

using namespace renderlib;
using namespace glm;

class Trackball {
	
    public:
        Trackball(float width, float height, float radius);
        void MouseDown(int x, int y);
        void MouseUp(int x, int y);
        void MouseMove(int x, int y);
        void ReturnHome();
        mat3 GetRotation() const;
        void Update(unsigned int microseconds);
        float GetZoom() const;
    private:
        vec3 MapToSphere(int x, int y);
        vec3 m_startPos;
        vec3 m_currentPos;
        vec3 m_previousPos;
        vec3 m_axis;
        quat m_quat;
        bool m_active;
        float m_radius;
        float m_radiansPerSecond;
        float m_distancePerSecond;
        float m_width;
        float m_height;
        float m_zoom;
        float m_startZoom;
        int m_startY;
        unsigned int m_currentTime;
        unsigned int m_previousTime;

        struct VoyageHome {
            bool Active;
            quat DepartureQuat;
            float DepartureZoom;
            unsigned int microseconds;
        } m_voyageHome;

        struct Inertia {
            bool Active;
            vec3 Axis;
            float RadiansPerSecond;
            float DistancePerSecond;
        } m_inertia;

	 glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) const;
};

#endif //RENDERLIB_TRACKBALL_H