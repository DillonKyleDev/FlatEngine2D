#pragma once
#include "Vector4.h"
#include "Component.h"


namespace FlatEngine 
{
	class Camera : public Component
	{
	public:
		Camera(long myID = -1, long parentID = -1);		
		~Camera();
		std::string GetData();

		void SetPrimaryCamera(bool b_isPrimary);
		bool IsPrimary();
		void SetDimensions(float width, float height);
		void SetZoom(float zoom);
		float GetZoom();
		float GetWidth();
		float GetHeight();
		void SetFrustrumColor(Vector4 color);
		Vector4 GetFrustrumColor();
		void Follow();
		void SetShouldFollow(bool b_shouldFollow);
		bool GetShouldFollow();
		void SetToFollowID(long toFollow);
		long GetToFollowID();
		void SetFollowSmoothing(float smoothing);
		float GetFollowSmoothing();

	private:
		float m_width;
		float m_height;
		float m_zoom;
		Vector4 m_frustrumColor;
		bool m_b_isPrimaryCamera;
		bool m_b_shouldFollow;
		long m_toFollowID;
		float m_followSmoothing;
	};
}