﻿#pragma once

#include "asd.Joystick.h"

namespace asd
{
	class Joystick_Imp
		: public Joystick
	{
	private:
		ap::Joystick*	joystick = nullptr;

		int m_connectId;
		asd::astring m_joystickName;

		Joystick_Imp(int id, ap::Joystick* joystick);
	public:
		virtual ~Joystick_Imp();
		static Joystick_Imp* Create(int id, ap::Joystick* joystick);

		const asd::achar* GetJoystickName() override;
		int GetButtonsCount() override;
		int GetAxesCount() override;
		JoystickButtonState GetButtonState(int at) override;
		float GetAxisState(int at) override;
	};
};
