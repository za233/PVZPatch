#include"offset.h"
#include<windows.h>
#define ReadDWORD(obj,offset) *(PDWORD)(obj+offset)
#define WriteDWORD(obj,offset,val) *(PDWORD)(obj+offset)=(DWORD)val
#define ReadFLOAT(obj,offset) *(float*)(obj+offset)
#define WriteFLOAT(obj,offset,val) *(float*)(obj+offset)=(float)val
struct CollisionBox
{
	int X;
	int Y;
	int Width;
	int Height;
};
struct List
{
	DWORD addr_list[100000];
	int size;
};
DWORD base,getPlant,getZombie,getAmmo,plantCall,shootCall,fireEffectCall,damageZombieCall,spawnZombieCall,getSpawnLineCall,findEnemy;
List plant_list,zombie_list,ammo_list;
bool has_zombie_line[7];
DWORD this_;
void LimboPage(DWORD base)
{
	DWORD prev,p;
	PBYTE addr1=(PBYTE)(base+0x4895A),addr2=(PBYTE)(addr1+1),addr3=(PBYTE)(addr1+2);
	VirtualProtect(addr1,3,PAGE_READWRITE,&prev);
	*addr1=0x90;
	*addr2=0x90;
	*addr3=0x90;
	VirtualProtect(addr1,3,prev,&p);
	MessageBoxA(NULL,"ÕýÔÚ¿ªÆô×´Ì¬Ò³","LimboPage",MB_OK);
}
PDWORD GetAmmoDamage(DWORD type)
{
	return (PDWORD)(0x76C550+type*12);
}
void GetCollisionBox(DWORD zombie,CollisionBox* box)
{
	box->X=ReadDWORD(zombie,0x8C);
	box->Y=ReadDWORD(zombie,0x90);
	box->Width=ReadDWORD(zombie,0x94);
	box->Height=ReadDWORD(zombie,0x98);
}

DWORD getRender(DWORD obj)
{
	DWORD func=0x18D10+base,addr;
	_asm
	{
		mov ebx,obj
		mov eax,[ebx]
		mov eax,[eax+0x944]
		mov esi,[ebx+0x94]
		mov edi,[eax+0x8]
		call func
		mov addr,eax
	}
	return addr;
}
void SetColor(DWORD obj,DWORD r,DWORD g,DWORD b,DWORD alpha)
{
	DWORD render=getRender(obj);
	WriteDWORD(render,72,r);
	WriteDWORD(render,76,g);
	WriteDWORD(render,80,b);
	WriteDWORD(render,84,alpha);
}
void DamageZombie(DWORD arg,DWORD zombie,DWORD dmg)
{
	_asm
	{
		mov eax,arg;
		mov esi,zombie
		push dmg
		call damageZombieCall
	}
}
void SpawnZombie(DWORD game,DWORD type0)
{
	_asm
	{
		mov esi,type0
		mov edi,game
		push esi
		mov eax,edi
		call getSpawnLineCall
		push eax
		push esi
		mov eax,edi
		call spawnZombieCall
	}
}
void FreezeZombieInLine(DWORD line)
{
	for(int i=0;i<zombie_list.size;i++)
	{
		DWORD addr=zombie_list.addr_list[i];
		if(ReadDWORD(addr,28)!=line)
			continue;
		WriteDWORD(addr,ZOMBIE_FREEZE,300);
		WriteDWORD(addr,ZOMBIE_ICE_SLOW,2000);
	}
}
void WriteToDBGFile(char *str)
{
	FILE *fp=fopen("dbg.txt","a+");
	if(fp!=NULL)
	{
		fprintf(fp,"%s",str);
		fclose(fp);
	}
}
void SeduceZombie(DWORD zombie)
{
	WriteDWORD(zombie,SEDUCE_FLAG,1);
}
double getDistance(double x1,double y1,double x2,double y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
bool HasZombieInLine(DWORD line)
{
	return has_zombie_line[line];
}
void DamageZombieInLine(DWORD arg,DWORD line,DWORD dmg)
{
	for(int i=0;i<zombie_list.size;i++)
	{
		DWORD addr=zombie_list.addr_list[i];
		if(ReadDWORD(addr,28)!=line)
			continue;
		DamageZombie(arg,addr,dmg);
	}
}
void SetAmmoTarget(DWORD game,FLOAT speed,DWORD ammo,DWORD zombie)
{
	WriteFLOAT(ammo,60,speed);
	WriteDWORD(ammo,88,9);
	DWORD target,func=base+0x35020;
	_asm
	{
		mov eax,game
		mov ebx,zombie
		mov ecx,func
		call ecx
		mov target,eax
	}
	WriteDWORD(ammo,136,target);
}
void EqrthQuake(DWORD game,DWORD duration)
{
	WriteDWORD(game,GAME_EARTHQUAKE_DURATION,duration);
	WriteDWORD(game,GAME_EARTHQUAKE_HORIZONTAL,2);
	WriteDWORD(game,GAME_EARTHQUAKE_VERTICAL,4);
}
void FlyZombieInLine(DWORD line)
{
	for(int i=0;i<zombie_list.size;i++)
	{
		DWORD addr=zombie_list.addr_list[i];
		if(ReadDWORD(addr,28)!=line)
			continue;
		if(ReadDWORD(addr,ZOMBIE_TYPE)>=3)
			continue;
		WriteDWORD(addr,ZOMBIE_FLY_AWAY,1);
	}
}
DWORD GetGamePass(DWORD game)
{
	return ReadDWORD(game,GAME_SCENE);
}
void ShootAmmo(DWORD plant,DWORD arg,DWORD line,DWORD flag)
{
	_asm
	{
		push flag
		push line
		push arg
		push plant
		call shootCall
	}
}
DWORD PlantCall(DWORD game,DWORD line,DWORD column,DWORD plant_type)
{
	DWORD plant;
	_asm
	{
		push 0xFFFFFFFF
		push plant_type
		mov eax,line
		push column
		mov edi,game
		push edi
		call plantCall
		mov plant,eax
	}
	return plant;
}
void PlantEffect(DWORD plant)
{
	DWORD func=base+0x8A950;
	_asm
	{
		push plant
		call func
	}
}
void DoomShroomEffect(DWORD game,DWORD line,DWORD column)
{
	DWORD add=PlantCall(game,line,column,15);
	PlantEffect(add);
}
DWORD FindEnemy(DWORD _this,DWORD plant,DWORD line)
{
	DWORD ans;
	_asm
	{
		push line
		push plant
		mov ecx,_this
		call findEnemy;
		mov ans,eax
	}
	return ans;
}
void KillPlant(DWORD addr)
{
	WriteDWORD(addr,PLANT_HP,-1);
}
void CherryBombExplode(DWORD game,DWORD line,DWORD column)
{
	DWORD bomb=PlantCall(game,line,column,2);
	WriteDWORD(bomb,PLANT_ONCE_TIMER,1);
}
void JalapenoExplode(DWORD game,DWORD line,DWORD column)
{
	DWORD bomb=PlantCall(game,line,column,20);
	WriteDWORD(bomb,PLANT_ONCE_TIMER,1);
}
void FireLineEffect(DWORD _esi,DWORD game,DWORD line)
{
	_asm
	{
		mov esi,_esi
		push line
		push game
		call fireEffectCall
	}
}
