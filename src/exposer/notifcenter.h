#include <Poco/Notification.h>
#include <Poco/NotificationCenter.h>
#include <Poco/AutoPtr.h>

#include "gamedata.h"

class NotifCenter {
	public:
		static Poco::NotificationCenter static_nc;
};

class BaseNotification : public Poco::Notification
{
public:
	void load(GameData passed_data);
	GameData data;
private:

};
class SubNotification : public BaseNotification
{

};

class Target
{
public:
	void handleBase(BaseNotification* pNf);
//	void handleSub(const Poco::AutoPtr<SubNotification>& pNf);
};
