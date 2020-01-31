// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include<cstdlib>
#include<ctime>
#include"Timer.h"
#include"Type.h"
#include"MoreZombie.h"
#include<windows.h>
#ifdef API
#elif
#include"API.h"
#endif
bool debug=false;   //开启无线阳光 方便调试
void UnlimitedSun(DWORD _edi)
{
	if(_edi==NULL)
		return;
	*(PDWORD)(_edi+0x5578)=9990;
}
void TickHandler(DWORD _edi)   //plant+4就是这个edi，也许代表的是game对象
{
	if(_edi==NULL)
		return;
	int direct[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}};
	int boundx=0,boundy=0;
	int now=time(0);
	DWORD gmpass=GetGamePass(_edi);
	if(gmpass<0)
		return;
	if(gmpass<=1 || (gmpass>=4 && gmpass<=5))
		boundy=8,boundx=4;
	if(gmpass>=2 && gmpass<=3)
		boundy=8,boundx=5;
	for(int i=0;i<plant_list.size;i++)
	{
		DWORD addr=plant_list.addr_list[i];
		if(addr==NULL)
			continue;
		DWORD type=ReadDWORD(addr,PLANT_TYPE);
		DWORD line=ReadDWORD(addr,PLANT_LINE),column=ReadDWORD(addr,PLANT_COLUMN);
		int r=rand()%1000;
		if(ReadDWORD(addr,PLANT_HP)>=0 && ReadDWORD(addr,PLANT_HP)<=50)
		{
			switch(type)
			{
				case 1:
					if(r<=5)
					{
						
						PlantCall(_edi,line,column,51);
						KillPlant(addr);
					}
					break;
				case 51:
					CherryBombExplode(_edi,line,column);
					PlantCall(_edi,line,column,50);
					KillPlant(addr);
					break;
				case 3:
				case 23:
					WriteDWORD(addr,PLANT_TYPE,17);
					break;
				case 22:
					FireLineEffect(65,_edi,line);
					FreezeZombieInLine(line);
					DamageZombieInLine(1,line,100);
					//JalapenoExplode(_edi,line,column);
					KillPlant(addr);
					break;
				case 31:
				case 45:
					CherryBombExplode(_edi,line,column);
					KillPlant(addr);
					break;
			}
			
		}
		if(ReadDWORD(addr,PLANT_TYPE)==6)
		{
			if(ReadDWORD(addr,60)==0xA)
				DamageZombieInLine(1,ReadDWORD(addr,PLANT_LINE),2);
				
		}
		if(ReadDWORD(addr,PLANT_TYPE)==4 && ReadDWORD(addr,60)==16 && FindEnemy(0,addr,line))
		{
			PlantEffect(addr);
			DoomShroomEffect(_edi,line,column);
		}
		if(ReadDWORD(addr,PLANT_ONCE_TIMER)==2)
		{
			switch(type)
			{
				case 2:
					for(int i=0;i<4;i++)
					{
						int tx=direct[i][0]+line,ty=direct[i][1]+column;
						if(tx>=0 && ty>=0 && tx<=boundx && ty<=boundy)
							CherryBombExplode(_edi,tx,ty);
					}
					break;
				case 20:
					CherryBombExplode(_edi,line,column);
					break;
				case 15:
					for(int i=0;i<zombie_list.size;i++)
					{
						DWORD addr=zombie_list.addr_list[i];
						SeduceZombie(addr);
					}
					break;
				case 27:
					FlyZombieInLine(line);
					break;
			}
		}
	}
	if(ammo_list.size>1000)
		return;
	for(int i=0;i<ammo_list.size;i++)
	{
		DWORD addr=ammo_list.addr_list[i];
		if(addr==NULL || ReadDWORD(addr,AMMO_TYPE)!=CARAMBOLA)
			continue;
		if(fabs(ReadFLOAT(addr,AMMO_X_VECTOR)-4.0)>1e-4)
			WriteFLOAT(addr,AMMO_X_VECTOR,0.0);
		if(fabs(ReadFLOAT(addr,AMMO_Y_VECTOR)-4.0)>1e-4)
			WriteFLOAT(addr,AMMO_Y_VECTOR,0.0);
		if(now%15==0 && HasZombieInLine(ReadDWORD(addr,AMMO_LINE)))
		{
			WriteFLOAT(addr,AMMO_Y_VECTOR,4);
			continue;
		}
	}
	
}
void AmmoSpawnHandler(DWORD _eax,DWORD arg1,DWORD arg2,DWORD arg3,DWORD line,DWORD ammo_type)
{
	if(!(*((PWORD)ammo_type)))
	{
		int x=rand()%100;
		if(x<=30)
			*((PWORD)ammo_type)=0;
		else if(x<=48)
			*((PWORD)ammo_type)=8;
		else if(x<=53)
		{
			if(rand()%100<=50)
				*((PWORD)ammo_type)=9;
		}
		else if(x<=60)
			*((PWORD)ammo_type)=3;
		else if(x<=85)
			*((PWORD)ammo_type)=2;
		else if(x<=87)
			*((PWORD)ammo_type)=12;
	}
	if(*((PWORD)ammo_type)==CORN)
	{
		if(rand()%100>90)
			*((PWORD)ammo_type)=CORN_BOMB;
	}
}
void CollideHandler(DWORD zombie,DWORD ammo)
{
	DWORD type=ReadDWORD(ammo,AMMO_TYPE);
	switch(type)
	{
		case ICE_PEA:
			if(zombie==NULL)
				break;
			WriteDWORD(zombie,ZOMBIE_FREEZE,50);
			break;
		case BALL:
			if(zombie==NULL)
				break;
			if(ReadDWORD(zombie,ZOMBIE_TYPE)!=23 && ReadDWORD(zombie,ZOMBIE_TYPE)!=20 && ReadDWORD(zombie,ZOMBIE_TYPE)!=12)
				WriteDWORD(zombie,ZOMBIE_FLY_AWAY,1);
			break;
		case CARAMBOLA:
			if(zombie==NULL)
				break;
			DamageZombie(1,zombie,5);
			break;
		case SPORE:
			if(zombie==NULL)
				break;
			WriteDWORD(zombie,ZOMBIE_ICE_SLOW,250);
			DamageZombie(1,zombie,10);
			break;
	}
}
void __stdcall FindZombieCallBack(bool *flag,PDWORD zombie,DWORD plant,DWORD line)
{
	if(ReadDWORD(plant,PLANT_TYPE)==6)
	{
		*flag=1;
		for(int i=0;i<zombie_list.size;i++)
		{
			DWORD addr=zombie_list.addr_list[i];
			if(ReadDWORD(addr,ZOMBIE_LINE)==ReadDWORD(plant,PLANT_LINE) && ReadDWORD(addr,ZOMBIE_STATE)==0 && ReadDWORD(addr,0x18) && ReadDWORD(addr,ZOMBIE_FLY_AWAY)!=1)
				*zombie=addr;
			break;
		}
	}
	return;
}
void PlantDamagedHandler(DWORD plant,DWORD zombie)
{
	if(zombie==NULL || plant==NULL)
		return;
	DWORD type=ReadDWORD(plant,PLANT_TYPE);
	if(type==1)
		DamageZombie(1,zombie,2);
	if(type==36 || type==9)
		DamageZombie(1,zombie,5);
}
extern "C" __declspec(dllexport) void __stdcall PlantColor() //未开发
{
	DWORD _ebx,_edi;
	_asm
	{
		mov eax,[ebp+0x4*7]
		mov _ebx,eax
		mov eax,[ebp+0x4*3]
		mov _edi,eax
	}
	DWORD plant=_edi,render=_ebx;
	if(_edi==NULL)
		return;
	if((ReadDWORD(plant,PLANT_TYPE)==1 || ReadDWORD(plant,PLANT_TYPE)==9) && ReadDWORD(plant,PLANT_HP)>=0 && ReadDWORD(plant,PLANT_HP)<255)
		SetColor(plant,255,255,255,ReadDWORD(plant,PLANT_HP));
	if(ReadDWORD(plant,PLANT_TYPE)==3 && ReadDWORD(plant,PLANT_HP)>=0)
	{
		DWORD hp=ReadDWORD(plant,PLANT_HP);
		int x=255*hp/4000;
		SetColor(plant,255,x,x,255);
	}
	if(ReadDWORD(plant,PLANT_TYPE)==6 && ReadDWORD(plant,84)>=0 && ReadDWORD(plant,84)<=500)
	{
		DWORD x=ReadDWORD(plant,84)*255/500;
		SetColor(plant,255,255,x,255-x);
	}
}
extern "C" __declspec(dllexport) void __stdcall PlantDamaged()  
{
	DWORD _eax,_ecx,_edx,_ebx,_esi,_edi;
	_asm
	{
		mov eax,[ebp+0x4*10]
		mov _eax,eax
		mov eax,[ebp+0x4*9]
		mov _ecx,eax
		mov eax,[ebp+0x4*8]
		mov _edx,eax
		mov eax,[ebp+0x4*7]
		mov _ebx,eax
		mov eax,[ebp+0x4*3]
		mov _edi,eax
		mov eax,[ebp+0x4*4]
		mov _esi,eax
	}
	DWORD zombie;
	_asm
	{
		mov eax,[ebp]
		mov zombie,eax
	}
	DWORD plant=_esi;
	PlantDamagedHandler(plant,zombie);
}

extern "C" __declspec(dllexport,naked) void __stdcall FindZombie()
{
	_asm
	{
		push ebp
		mov ecx,ebp
		mov ebp,esp
		sub esp,24h
	}
	bool flag;
	DWORD plant,line;
	DWORD zombie;
	_asm 
	{
		mov eax,[ecx+0x8]
		mov plant,eax
		mov eax,[ecx+0xC]
		mov line,eax
		mov zombie,0
	}
	FindZombieCallBack(&flag,&zombie,plant,line);
	if(flag)
	{
		_asm mov eax,zombie
		_asm mov ebx,1
	}
	else
		_asm mov ebx,0
	_asm
	{
		mov esp,ebp
		pop ebp
		retn
	}
		
}
extern "C" __declspec(dllexport) void __stdcall AmmoCollide()   //弹体碰撞
{
	DWORD _eax,_ecx,_edx,_ebx,_esi,_edi;
	_asm
	{
		mov eax,[ebp+0x4*10]
		mov _eax,eax
		mov eax,[ebp+0x4*9]
		mov _ecx,eax
		mov eax,[ebp+0x4*8]
		mov _edx,eax
		mov eax,[ebp+0x4*7]
		mov _ebx,eax
		mov eax,[ebp+0x4*3]
		mov _edi,eax
		mov eax,[ebp+0x4*4]
		mov _esi,eax
	}
	DWORD zombie=_esi,ammo=_edi;
	CollideHandler(zombie,ammo);
}
extern "C" __declspec(dllexport) void __stdcall TickFunc()  //游戏时循坏调用
{
	
	DWORD _eax,_ecx,_edx,_ebx,_esi,_edi;
	bool has_plant,has_zombie,has_ammo;
	memset(has_zombie_line,false,sizeof(has_zombie_line));
	_asm
	{
		mov eax,[ebp+0x4*10]
		mov _eax,eax
		mov eax,[ebp+0x4*9]
		mov _ecx,eax
		mov eax,[ebp+0x4*8]
		mov _edx,eax
		mov eax,[ebp+0x4*7]
		mov _ebx,eax
		mov eax,[ebp+0x4*3]
		mov _edi,eax
		mov eax,[ebp+0x4*4]
		mov _esi,eax
	}
	this_=_ecx;
	DWORD p=0;
	plant_list.size=0;
	while(1)         //获取植物
	{
		_asm
		{
			mov edx,_edi
			lea esi,[p]
			call getPlant
			mov has_plant,al
		}
		if(!has_plant)
			break;
		plant_list.addr_list[plant_list.size++]=p;
	}
	p=0;
	zombie_list.size=0;
	while(1)       //获取僵尸
	{ 
		_asm
		{
			mov edx,_edi
			lea esi,[p]
			call getZombie
			mov has_zombie,al
		}
		if(!has_zombie)
			break;
		zombie_list.addr_list[zombie_list.size++]=p;
		has_zombie_line[ReadDWORD(p,ZOMBIE_LINE)]=true;
	}
	p=0;
	ammo_list.size=0;
	while(1)       //获取弹体
	{
		_asm
		{
			mov edx,_edi
			lea esi,[p]
			call getAmmo
			mov has_ammo,al
		}
		if(!has_ammo)
			break;
		ammo_list.addr_list[ammo_list.size++]=p;
	}
	TickHandler(_edi);
	TimerTick();
	ZombieSpawnTimer(_edi);
	if(debug)
		UnlimitedSun(_edi);
}
extern "C" __declspec(dllexport) void __stdcall AmmoSpawn()
{
	DWORD ammo_type,line,_eax,arg1,arg2,arg3;
	_asm
	{
		mov _eax,eax
		lea eax,[ebp+0x48]
		mov ammo_type,eax
		lea eax,[ebp+0x44]
		mov line,eax
		lea eax,[ebp+0x40]
		mov arg3,eax
		lea eax,[ebp+0x3C]
		mov arg2,eax
		lea eax,[ebp+0x38]
		mov arg1,eax
	}
	AmmoSpawnHandler(_eax,arg1,arg2,arg3,line,ammo_type);
}
BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{		
	if(!base)
	{
		srand(time(0));
		HANDLE h=GetModuleHandle(NULL);
		SpawnTableInit();
		base=(DWORD)h;
		getPlant=base+0x351D0;
		getZombie=base+0x35170;
		getAmmo=base+0x35230;
		plantCall=base+0x22610;
		shootCall=base+0x8B190;
		fireEffectCall=base+0x35C50;
		damageZombieCall=base+0x167160;
		spawnZombieCall=base+0x236D0;
		getSpawnLineCall=base+0x23570;
		findEnemy=base+0x8B8D0;
		LimboPage(base);
	}
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		MessageBoxA(NULL,"PVZ_Patched by SomeOne just for Amusement","PVZ Version RG",MB_OK);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}