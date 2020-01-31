#include<windows.h>
#include<queue>
#include<ctime>
#include"API.h"
#define API
using namespace std;
#define DELAY_NORMAL_SHOOT 1
#define DELAY_ONCE_PLANT 2
#define DELAY_NORMAL_PLANT 3
#define DELAY_KILL_PLANT 4
#define GetTime() time(0)
long long last_time=0;
struct Time_Quest
{
	long long time;
	int action;
	DWORD arg0=NULL,arg1=NULL,arg2=NULL,arg3=NULL,arg4=NULL;
	Time_Quest(long long a,int b){time=a,action=b;}
	bool operator<(const Time_Quest& a) const
	{
		return time>a.time; 
	}
};
priority_queue<Time_Quest> quests;
void HandleCall(Time_Quest fp)
{
	switch(fp.action)
	{
		case DELAY_NORMAL_SHOOT:
			ShootAmmo(fp.arg0,fp.arg1,fp.arg2,fp.arg3);
			break;
		case DELAY_NORMAL_PLANT:
			PlantCall(fp.arg0,fp.arg1,fp.arg2,fp.arg3);
			break;
		case DELAY_KILL_PLANT:
			KillPlant(fp.arg0);
			break;
		case DELAY_ONCE_PLANT:
			DWORD obj=PlantCall(fp.arg0,fp.arg1,fp.arg2,fp.arg3);
			WriteDWORD(obj,PLANT_ONCE_TIMER,1);
			break;
	}
}
void TimerTick()
{
	long long now=GetTime();
	if(now-last_time<=0)
		return;
	last_time=now;
	if(quests.empty())
		return;
	Time_Quest fp=quests.top();
	while(now>=fp.time)
	{
		HandleCall(fp);
		quests.pop();
		if(quests.empty())
			break;
		fp=quests.top();
	}
	
}
bool AddDelayCall(Time_Quest quest)
{
	if(quests.size()>=100)
		return false;
	quests.push(quest);
	return true;
}
