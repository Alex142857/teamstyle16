#include "basic.h"
#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>
#include<cmath>
#include<map>
#include<vector>
#include<algorithm>

int MyTactics=0;//{�����ڼ���AIս���� 0=�վ�����+������  1=�վ�ѹ����  2=��ĸǱͧѹ����  3=��ĸǱͧһ����  4=����ĸһ����  5=����ĸѹ����}
int AI_Begin=false;

// Remove this line if you konw C++, and don't want a dirty namespace
using namespace teamstyle16;
using std::min;
using std::max;
using std::vector;
using std::string;
using std::ofstream;
using std::map;
using std::swap;
using std::cout;
using std::endl;
using std::sort;

//ofstream output("DEBUG.txt");
const int INF=10000; //INFΪ10000
const char * GetTeamName()//���س������ַ�ָ��Ķ�������
{
    return "ShangShi_44_Kill";  // Name of your team
}

int f(int a){return a<0?INF:a;}//�����������ʵֵ(-1��INF)
struct coor
{
	coor(int a=0,int b=0,int c=0):x(a),y(b),z(c){}
	coor(const Position &a):x(a.x),y(a.y),z(a.z){}
	int x,y,z;
};
bool operator<(const coor &a,const coor &b)
{
	if(a.x!=b.x)return a.x<b.x;
	if(a.y!=b.y)return a.y<b.y;
	if(a.z!=b.z)return a.z<b.z;
	return false;
}

const GameInfo *INFO = Info();//���������ɶ�ģ�

//����
const int map_size_max = 110;//��ͼ��С
const int Move_Offset[4][2]={1,0,0,1,-1,0,0,-1};//����ͨƫ����
const int Priority_Order[10]={7,4,8,9,5,6,0,1,2,3};//��λ�������ȼ�
int Element_Max[10]={1,0,0,0,14,0,6,8,17,1};//��λ��������
int LowerFuel[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};//��λ�ӻ��س�������Я����ȼ����
const int Min_Weather=-4;//����С�ڴ�ʱǿ�Ʊ���һ������

//���ñ���
int MyTeam,EnemyTeam;//���Ҷ�����
const State *MyBase,*EnemyBase;
coor MyBase_Coor,EnemyBase_Coor;//����˫����������
int MyBase_ID;//�ҷ�������INFO->elements�еı��
int MyBase_Index,EnemyBase_Index;//����˫�����ؽṹ��
int DistToEnemyPort,DistToEnemyFireRange;//�ҷ��������з�����/��ĸ������Χ��Զ���·
int My_Element_Now[10];//��ǰ��ͼ��������еĲ�����
vector<coor>MyPort,MyAirport,EnemyPort,EnemyAirport;//�ۿڣ��ո�����

//���ú���
bool check(const coor &a){return a.x>=0&&a.y>=0&&a.x<INFO->x_max&&a.y<INFO->y_max;}//�������a�Ƿ��ڵ�ͼ��
int distance(const State *p,const State *q)//����������λ֮��ľ���
{
	if(p->type == BASE || q->type == BASE)
	{
		return 
			((p->pos.x <  q->pos.x)?q->pos.x-p->pos.x-1:((p->pos.x >  q->pos.x)?p->pos.x-q->pos.x-1:0)) +
			((p->pos.y <  q->pos.y)?q->pos.y-p->pos.y-1:((p->pos.y >  q->pos.y)?p->pos.y-q->pos.y-1:0));
	}
	else return abs(p->pos.x - q->pos.x) + abs(p->pos.y - q->pos.y);
}
int distance(coor pos1, coor pos2)//����֮��ľ���
{
	return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}
int DisToBase(coor pos1, coor pos2)//���ص����صľ���
{
	return 
		((pos1.x <  pos2.x)?pos2.x-pos1.x-1:((pos1.x >  pos2.x)?pos1.x-pos2.x-1:0)) +
		((pos1.y <  pos2.y)?pos2.y-pos1.y-1:((pos1.y >  pos2.y)?pos1.y-pos2.y-1:0));
}
int fire(int attack,int distance,int fire_range,bool isCarrier=false)//�������㹫ʽ
{
	if(distance>fire_range)return 0;
	if(isCarrier)return f(attack);
	else return int((1 - float(distance - fire_range / 2) / (fire_range + 1)) * attack);
}
int fire(const State *p,const State *q)//����p����q����ʵ�˺�(�����Ƿ��������)
{
	const Property *P=&kProperty[p->type],*Q=&kProperty[q->type];
	int dist=distance(p,q);
	if(dist > P->fire_ranges[Q->level])return 0;
	int once =
		max(0,fire(P->attacks[0],dist,P->fire_ranges[Q->level],p->type==CARRIER) - f(Q->defences[0])) +
		max(0,fire(P->attacks[1],dist,P->fire_ranges[Q->level],p->type==CARRIER) - f(Q->defences[1]));
	return once;
}

struct Shortest_Path_Struct
{
	int x,y,z;//��¼��ǰa,b�����Ǹ���Ϊ�������·
	int b[map_size_max][map_size_max];//���·����
	vector<coor>a;//�����ڵ㼯
	int operator[](const coor &a){return b[a.x][a.y];}
	Shortest_Path_Struct(int t=0x3f3f3f3f)
	{
		x=y=z=-1;
		for(int i=0;i<map_size_max;i++)
			for(int j=0;j<map_size_max;j++)
			b[i][j]=t;
	}
	void clear(int t=0x3f3f3f3f)
	{
		for(int i=0;i<map_size_max;i++)
			for(int j=0;j<map_size_max;j++)
			b[i][j]=t;
		while(a.size())a.pop_back();
	}
	void BFS(coor S,int Dist)//����S����뷶ΧDist�����·����b,�������꼯�ϴ���a
	{
		x=S.x;y=S.y;z=S.z;
		coor T;
		int tot=0,k;
		while(k=a.size())
		{
			b[a[k-1].x][a[k-1].y]=0x3f3f3f3f;
			a.pop_back();
		}
		b[S.x][S.y]=0;
		a.push_back(S);
		while(tot<(int)a.size())
		{
			S=a[tot++];
			if(b[S.x][S.y]<Dist)
			{
				for(k=0;k<4;k++)
				{
					T.x=S.x+Move_Offset[k][0];
					T.y=S.y+Move_Offset[k][1];
					T.z=S.z;
					if(check(T)&&(T.z==2||Map(T.x,T.y)==OCEAN)&&b[T.x][T.y]>INF)
					{
						a.push_back(T);
						b[T.x][T.y]=b[S.x][S.y]+1;
					}
				}
			}
		}
	}
	void Shortest_Path(coor S)
	{
		BFS(S,INF);
	}
	void Dist_To_Base(coor S,int D)//�����������ܱ�<=D��������һ�����·
	{
		coor T;
		int tot=0,k;
		while(k=a.size())
		{
			b[a[k-1].x][a[k-1].y]=0x3f3f3f3f;
			a.pop_back();
		}
		for(int i=0;i<INFO->x_max;i++)
			for(int j=0;j<INFO->y_max;j++)
			{
				T.x=i;
				T.y=j;
				T.z=S.z;
				if(DisToBase(S,T)<=D)
				{
					if(S.z==2 || Map(T.x,T.y) == OCEAN)
					{
						b[T.x][T.y]=0;
						a.push_back(T);
					}
				}
			}
		while(tot<(int)a.size())
		{
			S=a[tot++];
			for(k=0;k<4;k++)
			{
				T.x=S.x+Move_Offset[k][0];
				T.y=S.y+Move_Offset[k][1];
				T.z=S.z;
				if(check(T)&&(T.z==2||Map(T.x,T.y)==OCEAN)&&b[T.x][T.y]>INF)
				{
					a.push_back(T);
					b[T.x][T.y]=b[S.x][S.y]+1;
				}
			}
		}
	}
}
EnemyPortDist/*��о��ۿ����·*/,
MyPortDist/*���Ҿ��ۿ����·*/,
Base_Attack_Range/*��о�����8�������·*/,
MyAirportDist,/*���Ҿ��ո����·*/
EnemyAirportDist,/*��о��ո����·*/
StagingDist/*������������·*/,
SPS,MOVE_SPS/*��ʱ*/,
Cargo_Appointment;/*ÿ��λ���м������䴬ԤԼ*/;
struct Map_Struct
{
	int a[3][map_size_max][map_size_max];
	void clear(){memset(a,0,sizeof(a));}
	void Make_Power(int Team,AttackType T,const Property &D)//��������Team��������T�Ա���D�Ļ���ͼ����a
	{
		clear();
		const State *p;
		const Property *P;
		coor S;
		int t,l,i;
		for(t=0;t<INFO->element_num;t++)
		{
			p=INFO->elements[t];
			P=&kProperty[p->type];
			if(p->type!=OILFIELD && p->type!=MINE)
			if(p->team == Team || (Team==EnemyTeam && p->team==2))
			{
				S=p->pos;
				S.z=2;
				for(l=0;l<3;l++)
				{
					SPS.BFS(S,P->fire_ranges[l] + 2*(int)(p->type==0)-1);
					for(i=0;i<(int)SPS.a.size();i++)
					{
						int dist= (p->type==0)?DisToBase(SPS.a[i],p->pos):distance(p->pos,SPS.a[i]);
						if(dist <= P->fire_ranges[l])
							a[l][SPS.a[i].x][SPS.a[i].y]+=max(0,fire(P->attacks[T],dist,P->fire_ranges[l])-f(D.defences[T]));
					}
				}
			}
		}
	}
}EnemyFire[10]/*ÿ�����ֵĻ���ͼ*/,EnemyTorpedo[10]/*ÿ�����ֵ�����ͼ*/,Hinder,Hinder_Now;
struct Move_Manager
{
	
	void Move_Safety(const State *p,const Shortest_Path_Struct &B,bool Hinder_ifConsider = true)//��BΪ���·�����ذ�ȫ·���ƶ�p���Ƿ��Ƕ��ѿ�λ��
	{
		const Property *P=&kProperty[p->type];
		int i,j,k;
		Position Destination;
		MOVE_SPS.BFS(p->pos,P->speed);
		for(i=0,j=k=INF;i<(int)MOVE_SPS.a.size();i++)
		{
			coor T=MOVE_SPS.a[i];
			if(Hinder_ifConsider==false||!Hinder.a[T.z][T.x][T.y])
			{
				int dist = B.b[T.x][T.y]*2 + Hinder_Now.a[T.z][T.x][T.y] - (int)(p->pos.x==T.x&&p->pos.y==T.y);
				int hurt = EnemyFire[p->type].a[T.z][T.x][T.y] + EnemyTorpedo[p->type].a[T.z][T.x][T.y];
				if(hurt < j || (hurt == j && dist < k))
				{
					j=hurt;
					k=dist;
					Destination.x=T.x;
					Destination.y=T.y;
					Destination.z=T.z;
				}
			}
		}
		if(k<INF)
		{
			Hinder.a[Destination.z][Destination.x][Destination.y]++;
			ChangeDest(p->index,Destination);
		}
	}
	void Move_Straight(const State *p,const Shortest_Path_Struct &B,bool Hinder_ifConsider = true)//��BΪ���·����ֱ���ƶ�p���Ƿ��Ƕ��ѿ�λ��
	{
		int i,k;
		const Property *P=&kProperty[p->type];
		Position Destination;
		MOVE_SPS.BFS(p->pos,P->speed);
		for(i=0,k=INF;i<(int)MOVE_SPS.a.size();i++)
		{
			coor T=MOVE_SPS.a[i];
			if(Hinder_ifConsider==false||!Hinder.a[T.z][T.x][T.y])
			{
				int dist = B.b[T.x][T.y]*2 + Hinder_Now.a[T.z][T.x][T.y] - (int)(p->pos.x==T.x&&p->pos.y==T.y);//�����Ȩ���� ʵ�ʾ���*2+��ǰ���ڵ�λ*1-ԭ��ͣ��
				if(dist < k)
				{
					k=dist;
					Destination.x=T.x;
					Destination.y=T.y;
					Destination.z=T.z;
				}
			}
		}
		if(k<INF)
		{
			Hinder.a[Destination.z][Destination.x][Destination.y]++;
			ChangeDest(p->index,Destination);
		}
	}
}Move;
struct Resource_Manager//�����¼ά����ͼ����Դ�����Ϣ
{
	map<int,int>a/*ÿ��index��ʣ����Դ��*/;
	map<coor,int>id/*ÿ������ĵķ���ID*/;
	vector<Shortest_Path_Struct>SPS;//SPS[i]��¼�������ID=i�����·ͼ
	Shortest_Path_Struct* operator[](const coor &a){return &SPS[id[a]];}//���������������·ͼ��ָ��
	void init()//��Ϸ��ʼʱ��ʼ����ͼ
	{
		const State *p;
		//��ʼ����Դ��
		for(int i=0;i<INFO->element_num;i++)
		{
			const State *p=INFO->elements[i];
			if(p->type == 2)a[p->index] = 500;
			if(p->type == 3)a[p->index] = 1000;
		}
		//��ʼ��������ܱߵ����·
		int tot=0;
		coor S;
		for(int i=0;i<INFO->element_num;i++)
		{
			p=INFO->elements[i];
			if(p->type == 2 || p->type == 3)
			{
				for(int k=0;k<4;k++)
				{
					S.x=p->pos.x + Move_Offset[k][0];
					S.y=p->pos.y + Move_Offset[k][1];
					S.z=1;
					if(id.find(S)==id.end())
					{
						SPS.push_back(Shortest_Path_Struct());
						SPS[tot].BFS(S,INF);
						id[S]=tot++;
					}
				}
			}
		}
		//��ʼ�����ۿڵ����·
		for(int i=0;i<(int)MyPort.size();i++)
		{
			S=MyPort[i];
			if(id.find(S)==id.end())
			{
				SPS.push_back(Shortest_Path_Struct());
				SPS[tot].BFS(S,INF);
				id[S]=tot++;
			}
		}
	}
	void update()//������Ұ����Դ�����Ϣ
	{
		for(int i=0;i<INFO->element_num;i++)
		{
			const State *p=INFO->elements[i];
			if(p->visible == true)
			{
				if(p->type == 2)a[p->index] = p->metal;
				if(p->type == 3)a[p->index] = p->fuel;
			}
		}
	}
}Resource;//��ͼ��Դ������
struct Supply_Manager
{
	struct Demand{const State *p;int F,A,M;};
	vector<Demand>a,save;
	void clear(){while(a.size())a.pop_back();}
	void del(int t)//ɾ��a�б��t��Ԫ��
	{
		if(t<0||t>=(int)a.size())return;
		for(;t<(int)a.size()-1;t++)swap(a[t],a[t+1]);
		a.pop_back();
	}
	void add(const State *p,int F,int A=0,int M=0)//���һ������
	{
		Demand r;
		r.p=p;r.F=F;r.A=A;r.M=M;
		a.push_back(r);
	}
	Demand Supply_Dying(const State *p)//����p��Χ�����ص�λ�Ĳ�������,�����Ƿ���巽��(�޷����Ƿ���{-1,-1,-1})
	{
		Demand *t;
		int i,j,k=-1,Order;
		int Fuel_upper_bound=f(p->fuel);//�ܽ��ܵĶ���ȼ������Ͻ�
		int Ammo_upper_bound=f(p->ammo);//�ܽ��ܵĶ��ⵯҩ����Ͻ�
		for(i=0;i<(int)a.size();i++)
		{
			t=&a[i];
			int dist=(p->type==0)?DisToBase(t->p->pos,p->pos):distance(t->p->pos,p->pos);
			if(dist<=1)
			{
				if(Ammo_upper_bound>=t->A && Fuel_upper_bound>=t->F)//���Լ�������ʵʩ�����򲹸�
				{
					j=5*t->A+t->F;//���ȼ�
					if(j>k)
					{
						k=j;
						Order=i;
					}
				}
			}
		}
		if(k>=0)
		{
			t=&a[Order];
			del(Order);
			return *t;
		}
		Demand re;re.F=re.A=re.M=-1;
		return re;
	}
}Demand;//����/ά�������б�
struct Collect_Manager//�ɼ�AI������ͳ��󳵵��ƶ�·����ͨ��solve()��Ա��������
{
	map<int,coor>Move_Target;
	map<int,int>S_index,SF,SM,f_r;//index��S_index������fuel��metal��,��ȼ�ϲ��������󲹸���������Ϊf_r

	//���ࣻF,A,MΪ����Я��ȼ�ϣ���ҩ������������SF,SMΪ��S_index����������targetΪ��ǰ���ߵ�Ŀ�ĵ�,f_rΪ����������
	struct cargo{const State *p;int F,A,M,S_index,SF,SM,f_r;coor target;}Base/*����*/;
	struct resource{const State *p;int rest;vector<coor>a;};//��Դ���� restΪԤ��ʣ����Դ aΪ����Դ���ٺ�����
	vector<cargo>a;//��
	vector<resource>b;//��Դ��
	void clear()
	{
		while(a.size())a.pop_back();
		while(b.size())b.pop_back();
		Move_Target.clear();
		SF.clear();
		SM.clear();
		S_index.clear();
		f_r.clear();
	}
	void init()//����a��b
	{
		clear();
		coor T;T.z=1;
		for(int i=0;i<INFO->element_num;i++)
		{
			const State *p=INFO->elements[i];
			if((p->type==MINE || p->type==OILFIELD) && Resource.a[p->index]>0)//δ�ݽߵĿ󳡻�����
			{
				int k=b.size();
				b.push_back(resource());
				b[k].p=p;
				b[k].rest=Resource.a[p->index];
				for(int j=0;j<4;j++)
				{
					T.x=p->pos.x+Move_Offset[j][0];
					T.y=p->pos.y+Move_Offset[j][1];
					if(check(T) && Map(T.x,T.y)==OCEAN)
						b[k].a.push_back(T);
				}
			}
			else if(p->type==CARGO && p->team == MyTeam)
			{
				int k=a.size();
				a.push_back(cargo());
				a[k].p=p;
				a[k].F=p->fuel;
				a[k].A=p->ammo;
				a[k].M=p->metal;
			}
		}
	}
	void update()//����INFO����a,b����ĳ�ֵ,����ԭCargo_Appointment
	{
		Base.F=MyBase->fuel;
		Base.A=INF;
		Base.M=MyBase->metal;
		for(int i=0;i<(int)a.size();i++)
		{
			a[i].F=a[i].p->fuel;
			a[i].A=a[i].p->ammo;
			a[i].M=a[i].p->metal;
			a[i].SF=a[i].SM=0;
			a[i].S_index=-1;
			a[i].f_r=0;
		}
		for(int i=0;i<(int)b.size();i++)
		{
			b[i].rest=Resource.a[b[i].p->index];
			for(int j=0;j<(int)b[i].a.size();j++)
			Cargo_Appointment.b[b[i].a[j].x][b[i].a[j].y]=0;
		}
		for(int i=0;i<(int)MyPort.size();i++)
			Cargo_Appointment.b[MyPort[i].x][MyPort[i].y]=0;
	}
	int calc(int Pt1)//����a��˳��̰��Ѱ�����Ž�(��ʱa�еĵ�λȫ����Ҫ�ɼ�) Pt1Ϊ�˹�������
	{
		Demand.save=Demand.a;
		//ȱʲôȥ���ֿ���,������ȱ�Ļ�ö�ٶ�Ԫ��
		const int Speed = kProperty[CARGO].speed;
		cargo *A;
		int i,k,t;
		int n=a.size();
		int m=b.size();
		int re=0;//ÿ��һ�������ڻ����Ŷӿ۷�10000�������п󳵵��ۼ�����Ļغ���
		for(t=0;t<n;t++)
		{
			A=&a[t];
			//��Ϊ�˹����
			if(t<Pt1)
			{
				bool PF=false,PM=false;//��Χ���ڵĿ������Щ
				for(i=0;i<(int)b.size();i++)
					if(distance(A->p,b[i].p)<=1)
					{
						if(b[i].p->type==OILFIELD && b[i].rest>30)PF=true;
						else if(b[i].rest>0)PM=true;
					}
				int MAX=-1,SF=0,SM=0,order,id;//����Χ��Դ��ϡȱ�Ŀ󳵾�������
				for(i=0;i<n;i++)
					if(distance(a[i].p->pos,A->p->pos)==1)
					{
						k=(kProperty[CARGO].fuel_max - a[i].F)*PF + (kProperty[CARGO].metal_max - a[i].M)*PM*5;
						if(k>MAX)
						{
							MAX=k;
							SF=(kProperty[CARGO].fuel_max - a[i].F)*PF;
							SM=(kProperty[CARGO].metal_max - a[i].M)*PM;
							order=a[i].p->index;
							id=i;
						}
					}
				if(MAX>0)
				{
					A->target=A->p->pos;
					A->S_index=order;
					A->SM=SM;
					A->SF=SF;
					a[id].F+=A->SF;
					a[id].M+=A->SM;
					continue;
				}
			}

			//�Ŷ�����ز���

			//���Ƚ��в���
			/*Supply_Manager::Demand t=Demand.Supply_Dying(A->p);
			if(t.F!=-1)
			{
				//���Ƚ��в���
				re+=INF;
				A->target=A->p->pos;
				A->f_r=0;
				A->S_index=t.p->index;
				A->SF=t.F;
				A->SM=t.M;
				continue;
			}*/

			//�ƶ��ɼ�AI
			bool B1=false;//B1Ϊtrueʱ�ǲɼ���Ϊfalseʱ�ǻػ���
			int MIN=0x7fffffff;
			coor Target,Target2=coor(-1,-1,-1);
			resource *T1=NULL,*T2=NULL;
			int fuel_r=0;//ȼ������
			bool free_fuel=false;//����ɼ�ȼ��
			if(distance(A->p,MyBase)==1)//����ɼ�ȼ��
				free_fuel=true;
			if((A->F<=LowerFuel[CARGO]*2 || free_fuel) && A->M==0)//����Ҫ��ȼ������Ҫ�ɿ�
			{
				for(int o1=0;o1<m;o1++)
					if(b[o1].p->type==OILFIELD && b[o1].rest>=50)
						for(int o2=0;o2<(int)b[o1].a.size();o2++)
						{
							coor FC=b[o1].a[o2];//����Ŀ�ĵ�
							for(int m1=0;m1<m;m1++)
								if(b[m1].p->type==MINE && b[m1].rest>0)
									for(int m2=0;m2<(int)b[m1].a.size();m2++)
									{
										coor MC=b[m1].a[m2];
										{
											bool type=0;//Ϊ0���������󳡣�Ϊ1�෴
											
											int dist1= 2*(((*Resource[FC])[A->p->pos] + Speed - 1)/Speed + ( (*Resource[FC])[MC] + Speed - 1)/Speed + (MyPortDist[MC] + Speed - 1)/Speed) + Cargo_Appointment.b[FC.x][FC.y] + Cargo_Appointment.b[MC.x][MC.y]/2; //������
											int dist2= 2*(((*Resource[MC])[A->p->pos] + Speed - 1)/Speed + ( (*Resource[FC])[MC] + Speed - 1)/Speed + (MyPortDist[FC] + Speed - 1)/Speed) + Cargo_Appointment.b[FC.x][FC.y]/2 + Cargo_Appointment.b[MC.x][MC.y]; //�ȿ�
											if((*Resource[FC])[A->p->pos] > A->p->fuel && free_fuel==false)dist1=0x7fffffff;
											if((*Resource[MC])[A->p->pos] + (*Resource[FC])[MC] > A->p->fuel && free_fuel==false)dist2=0x7fffffff;
											int dist;
											if(dist1<=dist2)dist=dist1,type=0;
											else dist=dist2,type=1;
											if(dist<MIN)
											{
												MIN=dist;
												if(type==0)Target=FC,Target2=MC,T1=&b[o1],T2=&b[m1],fuel_r=(*Resource[FC])[A->p->pos];
												else Target=MC,Target2=FC,T1=&b[m1],T2=&b[o1],fuel_r=(*Resource[MC])[A->p->pos] + (*Resource[FC])[MC];
												B1=true;
											}
										}
									}
						}
				if(MIN==0x7fffffff)re-=INF;//����ʧ��
			}
			if((A->F<=LowerFuel[CARGO]*2 || free_fuel ) && MIN==0x7fffffff)//ֻ�����
			{
				for(int o1=0;o1<m;o1++)
					if(b[o1].p->type==OILFIELD && b[o1].rest>=50)
						for(int o2=0;o2<(int)b[o1].a.size();o2++)
						{
							coor FC=b[o1].a[o2];//����Ŀ�ĵ�
							int dist= 2*(((*Resource[FC])[A->p->pos] + Speed - 1)/Speed + (MyPortDist[FC] + Speed - 1)/Speed) + Cargo_Appointment.b[FC.x][FC.y];
							if(((*Resource[FC])[A->p->pos]<=A->F || free_fuel) &&dist<MIN)
							{
								MIN=dist;
								Target=FC;
								T1=&b[o1];
								fuel_r=(*Resource[FC])[A->p->pos];
								B1=true;
							}
						}
				if(MIN==0x7fffffff)re-=INF;//����ʧ��
			}
			if(A->M==0 && MIN==0x7fffffff)//ֻ��ɿ�
			{
				for(int o1=0;o1<m;o1++)
					if(b[o1].p->type==MINE && b[o1].rest>0)
						for(int o2=0;o2<(int)b[o1].a.size();o2++)
						{
							coor MC=b[o1].a[o2];//�ɿ�Ŀ�ĵ�
							int dist= 2*(((*Resource[MC])[A->p->pos] + Speed - 1)/Speed + (MyPortDist[MC] + Speed - 1)/Speed) + Cargo_Appointment.b[MC.x][MC.y]; 
							if(dist<MIN)
							{
								MIN=dist;
								Target=MC;
								T1=&b[o1];
								B1=true;
							}
						}
				if(MIN==0x7fffffff)re-=INF;//����ʧ��
			}
			if(MIN==0x7fffffff)//��Դ���㣬���ػ��ز���
			{
				for(i=0;i<(int)MyPort.size();i++)
				{
					int dist= (((*Resource[MyPort[i]])[A->p->pos] + Speed - 1)/Speed)*4 + Cargo_Appointment.b[MyPort[i].x][MyPort[i].y] - INF*(distance(A->p,MyBase)==1);
					if(dist<MIN)
					{
						MIN=dist;
						Target=MyPort[i];
					}
				}
				MIN=0;
			}

			//�޸�״̬
			if(free_fuel)//ͣ���ڻ��ظ������ǹ�������
			{
				//������������
				A->S_index=MyBase_Index;
				A->SM=min(200-Base.M,A->M);
				A->SF=max(0,A->F - fuel_r - 5);
			}
			if(free_fuel)//ȼ��δ����ز������(���ڻ�����������δ����������)��ǿ�Ʊ��ֲ���
			{
				if(A->SF + Base.F>1000)
				{
					A->SF=1000 - Base.F;
					Target=A->p->pos;
					Target2=coor(-1,-1,-1);
					B1=false;
					re-=100;//���������Ƶļӳ�
				}
				else if(A->F<fuel_r+5)
				{
					A->f_r=fuel_r+5 - A->F;
					Target=A->p->pos;
					B1=false;
					re-=1000;
				}
				Base.F+=A->SF;
				Base.M+=A->SM;
			}
			re+=MIN;
			A->M-=A->SM;
			A->F-=A->SF;
			A->target=Target;
			Cargo_Appointment.b[Target.x][Target.y]+=2;//ֱ��Ŀ�ĵؼ�����վλ
			if(Target2.z>=0)Cargo_Appointment.b[Target2.x][Target2.y]++;//���Ŀ�ĵؼ�һ��վλ
			if(B1)//ǰ���ɼ���״̬
			{
				if(T1->p->type==MINE)T1->rest -= min(T1->rest,50 - A->M);//�����Դ�۳�
				if(T1->p->type==OILFIELD)T1->rest -= min(T1->rest,200 - (A->F - fuel_r));//������Դ�۳�
				if(T2!=NULL)
				{
					if(T2->p->type==MINE)T2->rest -= min(T2->rest,50 - A->M);//�����Դ�۳�
					if(T2->p->type==OILFIELD)T2->rest -= min(T2->rest,200 - (A->F - fuel_r));//������Դ�۳�
				}
			}
			if(A->SF==0 && A->SM==0)A->S_index=-1;
		}
		Demand.a=Demand.save;
		return re;
	}
	void solve()//����ҷ��������䴬�Ĳ���
	{
		//����󳵵��������ž���
		//���϶�ʱ�ڻ�����Χ�ŶӵȺ򲹸�����
		//ͣ������Դ���ܱߵĿ󳵳䵱�˹���Դ��
		//��治�����������̰�ķ�Ѱ������·��(������ȼ�ϣ�����+ȼ��)

		init();
		//���̰�ķ�
		int i,j,k=0,n,m,cnt,MAX=-999999999;
		n=a.size();m=b.size();
		if(n*m==0)return;//û�п��з���
		cnt=100000/(n*m*m+n*n+10)+1;
		//cnt=1;//////DEBUG��
		while(cnt--)
		{
			//�����ɽ��
			for(i=1;i<n;i++)swap(a[rand()%(i+1)],a[i]);
			//��ͣ������Դ�ܱߵ����˹�������ȴ���
			for(i=k=0;i<n;i++)
				for(j=0;j<m;j++)
					if(distance(a[i].p,b[j].p)<=1)
					{
						swap(a[i],a[k++]);
						break;
					}

			update();//��ԭ��Ϣ
			if((j=calc(k))>MAX)
			{
				MAX=j;
				S_index.clear();
				SF.clear();
				SM.clear();
				f_r.clear();
				for(i=0;i<n;i++)
				{
					Move_Target[a[i].p->index]=a[i].target;
					if(a[i].S_index>=0)
					{
						S_index[a[i].p->index]=a[i].S_index;
						SF[a[i].p->index]=a[i].SF;
						SM[a[i].p->index]=a[i].SM;
					}
					if(a[i].f_r>0)f_r[a[i].p->index]=a[i].f_r;
				}
			}
		}
		Base.F=MyBase->fuel;
		Base.M=MyBase->metal;
		for(i=0;i<n;i++)//ִ�о���
		{
			//����λa[i].p->index �ƶ��� Move_Target[a[i].p->index]
			int Index=a[i].p->index;
			Move.Move_Straight(a[i].p,*Resource[Move_Target[a[i].p->index]],false);
			if(S_index.find(Index)!=S_index.end())
			{
				//����
				Supply(Index,S_index[Index],SF[Index],0,SM[Index]);
				if(S_index[Index]==MyBase->index)
				{
					Base.F+=SF[Index];
					Base.M+=SM[Index];
				}
			}
			else Demand.Supply_Dying(a[i].p);
			if(f_r[Index]>0)//���Ͳ�������
			{
				Demand.add(a[i].p,f_r[Index]);
			}
		}
	}
}Collect;
struct Attack_Manager//����AI������������Ź���Ŀ��(�����������ĵ�λpush������Ȼ����solve()ͳ�����)
{
	struct status{const State *p;int HP,target;};//��λ�࣬HPΪԤ��ʣ��Ѫ����targetΪ����Ŀ���index
	vector<status>a,b;//a�ҷ���������λ��b�з���λ
	map<int,int>Attack_Target;//�Ҿ�Index�����չ���Ŀ��Index
	void init()
	{
		clear();
		//�����ео���Ϣͳ����b
		for(int i=0;i<INFO->element_num;i++)
			if(INFO->elements[i]->team!=MyTeam && INFO->elements[i]->type!=OILFIELD && INFO->elements[i]->type!=MINE)
				for(int j=0;j<INFO->element_num;j++)
					if(INFO->elements[j]->team==MyTeam && fire(INFO->elements[j],INFO->elements[i])>0)
					{
						push(INFO->elements[i],true);
						break;
					}
	}
	void clear()
	{
		while(a.size())a.pop_back();
		while(b.size())b.pop_back();
		Attack_Target.clear();
	}
	void push(const State *p,bool T=false)//����λ�����ѡ�б���(Ĭ��Ϊa,T=true)
	{
		if(!T)
		{
			int k=a.size();
			a.push_back(status());
			a[k].p=p;
			a[k].HP=p->health;
			a[k].target=-1;
		}
		else
		{
			int k=b.size();
			b.push_back(status());
			b[k].p=p;
			b[k].HP=p->health;
			b[k].target=-1;
		}
	}
	bool check(const State *p)//����p�Ƿ���Ҫ���й���,����Ҫpush���������б�
	{
		const Property *P=&kProperty[p->type];
		if(f(p->ammo) < P->ammo_once)return false;
		//��������ео����빥��
		for(int i=0;i<(int)b.size();i++)
		{
			if(fire(p,b[i].p)>0)
			{
				push(p);
				return true;
			}
		}
		return false;
	}
	bool Attack_Dying(const State *p)//Ѱ�������������ĵ�λ����logic���͹���ָ��,�����Ƿ�ɹ�
	{
		const State *q,*re=NULL;
		const Property *P=&kProperty[p->type],*Q;
		if(f(p->ammo) < P->ammo_once)return false;
		int MIN=INF;
		for(int t=0;t<INFO->element_num;t++)
		{
			q=INFO->elements[t];
			Q=&kProperty[q->type];
			int dist= (p->type==0 || q->type==0)?DisToBase(p->pos,q->pos):distance(p->pos,q->pos);
			if(q->team == 1 - p->team && dist<=P->fire_ranges[Q->level])
			{
				int once =
					max(0,fire(P->attacks[0],dist,P->fire_ranges[Q->level],p->type==CARRIER) - f(Q->defences[0])) +
					max(0,fire(P->attacks[1],dist,P->fire_ranges[Q->level],p->type==CARRIER) - f(Q->defences[1]));
				if(once > 0)
				{
					int remain = (q->health + once -1) / once;
					if(remain < MIN)
					{
						MIN=remain;
						re=q;
					}
				}
			}
		}
		if(re!=NULL)
		{
			AttackUnit(p->index,re->index);
			return true;
		}
		return false;
	}
	int calc()//����a�������ڵ�˳��̰�ľ��߹���Ŀ�꣬�����ع�������������ֵ
	{
		//����ѡ�񹥻���Χ��������ĵ�λ��������������Χ�����е�λ�������������Ȳ��������һ��
		int i,j,n,m;
		int re=0;//����
		n=a.size();m=b.size();
		for(i=0;i<m;i++)b[i].HP=b[i].p->health;

		for(i=0;i<n;i++)
		{
			int MIN=2147483647,order=-1,MIN2;
			for(j=0;j<m;j++)
			{
				int once=fire(a[i].p,b[j].p);
				if(once>0)
				{
					int rest=(b[j].HP+once-1)/once;
					if(rest<=0)rest+=INF;//�õ�λ��������ʱ���ȹ��������һ��
					if(a[i].p->type==FIGHTER&&(b[j].p->type==FIGHTER||b[j].p->type==BASE))rest-=100;//�������ȹ����ɻ��ͻ���(�վ�һ�����ӳ�)
					if(rest<MIN||(rest==MIN&&once>MIN2))
					{
						MIN=rest;
						MIN2=once;
						order=j;
					}
				}
			}
			if(order>=0)
			{
				if(b[order].HP<=0)re++;//���Է���������Լ�˵�ҩ����+1
				b[order].HP-=fire(a[i].p,b[order].p);
				a[i].target=b[order].p->index;
			}else a[i].target=-1;
		}

		//����ֵ��ɱ��һ����λ����20�֣����һ����λ�۳�Ѫ����ֵ�ķ�
		for(i=0;i<m;i++)
			if(b[i].HP<=0)re+=20;
			else re-=b[i].HP;
		return re;
	}
	void solve()//���б��еĵ�λִ�о��߲���logic���Ͷ�������
	{
		//�������̰�ķ���������a�������ɴ�����Ų�������̰�Ľ�ȡ����
		int i,n,m,cnt,MAX=-999999999;
		n=a.size();m=b.size();
		if(n*m==0)return;//û�п��з���
		cnt=100000/(n*m*2)+1;
		while(cnt--)
		{
			//�����ɽ��
			for(i=1;i<n;i++)swap(a[rand()%(i+1)],a[i]);
			if(calc()>MAX)
				for(i=0;i<n;i++)Attack_Target[a[i].p->index]=a[i].target;
		}
		for(i=0;i<n;i++)
		{
			AttackUnit(a[i].p->index,Attack_Target[a[i].p->index]);
		}
	}
}Attack;
struct AI
{
	coor Staging;//���ӹ���ǰ�����
	map<int,int>Tactics;//����λ��ǰս��
	/*ս����
		ս����:{0//�������أ�1/����}
	*/
	void init()
	{
		//������Ұ����Դ������
		Resource.update();
		//���
		Hinder.clear();
		Hinder_Now.clear();
		Cargo_Appointment.clear(0);
		Demand.clear();
		Attack.init();
		Collect.init();
		//Ԥ����ȫͼ�Ļ���ֵ
		for(int i=0;i<10;i++)
		{		
			EnemyFire[i].Make_Power(EnemyTeam,FIRE,kProperty[i]);
			EnemyTorpedo[i].Make_Power(EnemyTeam,TORPEDO,kProperty[i]);
		}
		//�����ҷ��������� �� MyBase_ID MyBase EnemyBase
		memset(My_Element_Now,0,sizeof(My_Element_Now));
		for(int i=0;i<INFO->element_num;i++)
			if(INFO->elements[i]->team==MyTeam)
			{
				if(INFO->elements[i]->type==0)MyBase_ID=i,MyBase=INFO->elements[i];
				My_Element_Now[INFO->elements[i]->type]++;
			}
			else if(INFO->elements[i]->team==EnemyTeam)
			{
				if(INFO->elements[i]->type==0)EnemyBase=INFO->elements[i];
			}
		for(int i=0;i<INFO->production_num;i++)
			My_Element_Now[INFO->production_list[i].unit_type]++;
		//���������е�λ�ĸ��Ӵ���Hinder_Now
		for(int i=0;i<INFO->element_num;i++)
		{
			const State *p=INFO->elements[i];
			Hinder_Now.a[p->pos.z][p->pos.x][p->pos.y]=1;
		}

		//��EnemyAirportDist���м�Ȩ����
		EnemyAirportDist.Dist_To_Base(coor(EnemyBase_Coor.x,EnemyBase_Coor.y,2),0);
		for(int i=0;i<INFO->x_max;i++)
			for(int j=0;j<INFO->y_max;j++)
				EnemyAirportDist.b[i][j]*=2;
		for(int i=0;i<INFO->element_num;i++)
			if(INFO->elements[i]->type==FIGHTER)
			{
				if(distance(INFO->elements[i],EnemyBase)==0)
					EnemyAirportDist.b[INFO->elements[i]->pos.x][INFO->elements[i]->pos.y]+=INF;
			}

		//�޳�Tactics
		for(map<int,int>::iterator iter=Tactics.begin();iter!=Tactics.end();)
		{
			int i=0;
			for(i=0;i<INFO->element_num;i++)
				if(INFO->elements[i]->index == iter->first)
					break;
			if(i<INFO->element_num)iter++;
			else Tactics.erase(iter++);
		}
	}
	void DefenceStaging()//�����µķ�������ظ���Staging��StagingDist
	{
		const State *p;
		coor order;
		int MIN=0x7fffffff,i,j;
		for(i=0;i<INFO->element_num;i++)
		{
			p=INFO->elements[i];
			if(p->team==EnemyTeam && (j=distance(p,MyBase))<=8)
			{
				if(j<MIN)
				{
					MIN=j;
					order=p->pos;
				}
			}
		}
		if(MIN<0x7fffffff)
		{
			Staging=order;
			StagingDist.BFS(Staging,INF);
		}
	}
	void AI_Run()
	{
		//��ʼ��
		init();
		
		//�ɼ�����
		Collect.solve();
		
		//�������
		Produce_AI();
		
		//���÷��������
		DefenceStaging();

		//���������
		
		//���ÿ����λ������
		const State *p;
		const Property *P;
		for(int PO=0;PO<10;PO++)
		{
			int Type=Priority_Order[PO];
			for(int t=0;t<INFO->element_num;t++)
			{
				p=INFO->elements[t];
				P=&kProperty[p->type];
				if(p->type==Type&&p->team==INFO->team_num)
				{
					if(Type==BASE)Base_AI(p);
					if(Type==SUBMARINE)Submarine_AI(p);
					if(Type==CARRIER)Carrier_AI(p);
					//if(Type==CARGO)Cargo_AI(p);
					if(Type==FIGHTER)Fighter_AI(p);
					if(Type==FORT)Fort_AI(p);
					if(Type==SCOUT)Scout_AI(p);
				}
			}
		}
		Attack.solve();//���͹���ָ��
	}
	void AI_Start()//��սǰ��׼������(����һЩ����/����ṹ���ֵ)
	{
		const State *p;
		//������Ҷ����ţ����һ�������
		MyTeam=INFO->team_num;
		EnemyTeam=1-MyTeam;
		for(int i=0;i<INFO->element_num;i++)
		{
			p=INFO->elements[i];
			if(p->type == 0)
			{
				if(p->team == INFO->team_num)MyBase_Coor=p->pos,MyBase_Index=p->index,MyBase=p;
				else EnemyBase_Coor=p->pos,EnemyBase_Index=p->index,EnemyBase=p;
			}
		}
		//����EnemyPortDist,Base_Attack_Range
		EnemyPortDist.Dist_To_Base(EnemyBase_Coor,1);
		MyPortDist.Dist_To_Base(MyBase_Coor,1);
		Base_Attack_Range.Dist_To_Base(EnemyBase_Coor,8);
		EnemyAirportDist.Dist_To_Base(coor(EnemyBase_Coor.x,EnemyBase_Coor.y,2),0);
		MyAirportDist.Dist_To_Base(coor(MyBase_Coor.x,MyBase_Coor.y,2),0);
		//����DistToEnemyPort,DistToEnemyFireRange
		for(int i=0;i<INFO->x_max;i++)
			for(int j=0;j<INFO->y_max;j++)
				if(DisToBase(coor(i,j),MyBase_Coor)<=1&&Map(i,j)==OCEAN)
				{
					DistToEnemyPort=max(DistToEnemyPort,EnemyPortDist.b[i][j]);
					DistToEnemyFireRange=max(DistToEnemyFireRange,Base_Attack_Range.b[i][j]);
				}

		//����ۿ�/�ո�����
		coor S;
		for(S.x=0;S.x<INFO->x_max;S.x++)
			for(S.y=0;S.y<INFO->y_max;S.y++)
			{
				if(DisToBase(S,MyBase->pos)==1 && Map(S.x,S.y)==OCEAN)S.z=1,MyPort.push_back(S);
				if(DisToBase(S,EnemyBase->pos)==1 && Map(S.x,S.y)==OCEAN)S.z=1,EnemyPort.push_back(S);
				if(DisToBase(S,MyBase->pos)==0)S.z=2,MyAirport.push_back(S);
				if(DisToBase(S,EnemyBase->pos)==0)S.z=2,EnemyAirport.push_back(S);
			}

		//��ԭ��Դ������
		Resource.init();

		//���㵥λ�������ȼ�ϴ���
		LowerFuel[4]=min(120,DistToEnemyPort+5);
		LowerFuel[6]=min(200,DistToEnemyFireRange+20);
		LowerFuel[7]=(INFO->x_max+INFO->y_max)*5/12;
		LowerFuel[SCOUT]=kProperty[SCOUT].fuel_max-1;

		//��������س�ʼ��
		{
			int j=0x7fffffff,k;
			for(int i=0;i<(int)MyPort.size();i++)
			{
				if(EnemyPortDist.b[MyPort[i].x][MyPort[i].y]<j)
				{
					j=EnemyPortDist.b[MyPort[i].x][MyPort[i].y];
					k=i;
				}
			}
			Staging=MyPort[k];
			StagingDist.BFS(Staging,INF);
		}
	}
	void Produce_AI()
	{
		int i;
		int Metal_Rest=INFO->elements[MyBase_ID]->metal;//��ǰ����ʣ�����
		int Fuel_Rest=Collect.Base.F;//��ǰ����Ԥ��ʣ�����
		//�������۷�ֹ����ʻ��
		for(i=0;i<(int)MyPort.size();i++)
		{
			coor S=MyPort[i];
			Hinder.a[S.z][S.x][S.y]++;
			S.z=0;
			Hinder.a[S.z][S.x][S.y]++;
		}

		//���ٱ���1������
		if(INFO->weather<Min_Weather && My_Element_Now[SCOUT]<Element_Max[SCOUT] && Metal_Rest>=kProperty[SCOUT].cost)
		{
			Produce(SCOUT);
			My_Element_Now[SCOUT]++;
			Metal_Rest-=kProperty[SCOUT].cost;
		}
		
		if(MyTactics==4 || MyTactics==5 || (DistToEnemyPort>120 && distance(MyBase,EnemyBase)>80))
		{
			//����ĸһ����
			Element_Max[CARGO]=8-(INFO->weather<Min_Weather);
			
			//���Ƚ����䴬��������
			for(i=0;My_Element_Now[CARGO]<Element_Max[CARGO] && Metal_Rest>=kProperty[CARGO].cost;i++)
			{
				Produce(CARGO);
				My_Element_Now[CARGO]++;
				Metal_Rest-=kProperty[CARGO].cost;
			}

			//����ת��������������
			{
				int cnt=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam)
					{
						if(INFO->elements[i]->type==SUBMARINE && Tactics[INFO->elements[i]->index]==0)cnt+=kProperty[SUBMARINE].population;
						else if(INFO->elements[i]->type==CARRIER && Tactics[INFO->elements[i]->index]==0)cnt+=kProperty[CARRIER].population;
					}
				if(cnt>=60-Element_Max[CARGO]-3)//����50�˿ڽ�������
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && (INFO->elements[i]->type==SUBMARINE || INFO->elements[i]->type==CARRIER))
						Tactics[INFO->elements[i]->index]=1;
			}

			ElementType Type;
			if(INFO->round>2)
			while(INFO->population>55 || My_Element_Now[CARRIER]*4+My_Element_Now[CARGO]<=57)
			{
				Type=CARRIER;
				if(Metal_Rest>=kProperty[Type].cost)
				{
					Produce(Type);
					Metal_Rest-=kProperty[Type].cost;
					My_Element_Now[Type]++;
				}
				else break;
			}
			//�ں�ĸ��������ʱ�����ۿ�
			{
				int k=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==CARRIER)k++;
				int cnt=0;
				for(int i=0;i<INFO->production_num;i++)
				if(INFO->production_list[i].unit_type==CARRIER)
					if(INFO->production_list[i].round_left<2)cnt++;
				cnt=min(cnt,Fuel_Rest/kProperty[CARRIER].fuel_max);
				cnt=min(cnt,Element_Max[CARRIER]-k);
				for(int i=0;cnt>0&&i<(int)MyPort.size();i++)
				{
					for(k=0;k<INFO->element_num;k++)
						if(INFO->elements[k]->team == MyTeam && distance(INFO->elements[k]->pos,MyPort[i])==0)
							break;
					if(k<INFO->element_num || !Hinder_Now.a[MyPort[i].z][MyPort[i].x][MyPort[i].y])
					{
						Hinder.a[MyPort[i].z][MyPort[i].x][MyPort[i].y]++;
						cnt--;
					}
				}
			}
		}
		else
		//1��ĸ:3Ǳͧ ����ս��Ⱥ���� �˿���ʱ�ڽ����б���Ѻ�ĸ
		if(MyTactics==2 || MyTactics==3 || distance(MyBase,EnemyBase)>80)
		{
			Element_Max[CARGO]=9;
			
			//���Ƚ����䴬��������
			for(i=0;My_Element_Now[CARGO]<Element_Max[CARGO] && Metal_Rest>=kProperty[CARGO].cost;i++)
			{
				Produce(CARGO);
				My_Element_Now[CARGO]++;
				Metal_Rest-=kProperty[CARGO].cost;
			}

			//����ת��������������
			{
				int cnt=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam)
					{
						if(INFO->elements[i]->type==SUBMARINE && Tactics[INFO->elements[i]->index]==0)cnt+=kProperty[SUBMARINE].population;
						else if(INFO->elements[i]->type==CARRIER && Tactics[INFO->elements[i]->index]==0)cnt+=kProperty[CARRIER].population;
					}
				if(cnt>=60-Element_Max[CARGO]-2)//����50�˿ڽ�������
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && (INFO->elements[i]->type==SUBMARINE || INFO->elements[i]->type==CARRIER))
						Tactics[INFO->elements[i]->index]=1;
			}

			int K=2;
			ElementType Type;
			if(INFO->round>2)
			while(INFO->population>55 || My_Element_Now[CARRIER]*4+My_Element_Now[SUBMARINE]*2+My_Element_Now[CARGO]<=57)
			{
				if(My_Element_Now[CARRIER]*K<=My_Element_Now[SUBMARINE])Type=CARRIER;
				else Type=SUBMARINE;
				if(Metal_Rest>=kProperty[Type].cost)
				{
					Produce(Type);
					Metal_Rest-=kProperty[Type].cost;
					My_Element_Now[Type]++;
				}
				else break;
			}
			//�ں�ĸ��������ʱ�����ۿ�
			{
				int k=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==CARRIER)k++;
				int cnt=0;
				for(int i=0;i<INFO->production_num;i++)
				if(INFO->production_list[i].unit_type==CARRIER)
					if(INFO->production_list[i].round_left<2)cnt++;
				cnt=min(cnt,Fuel_Rest/kProperty[CARRIER].fuel_max);
				cnt=min(cnt,Element_Max[CARRIER]-k);
				for(int i=0;cnt>0&&i<(int)MyPort.size();i++)
				{
					for(k=0;k<INFO->element_num;k++)
						if(INFO->elements[k]->team == MyTeam && distance(INFO->elements[k]->pos,MyPort[i])==0)
							break;
					if(k<INFO->element_num || !Hinder_Now.a[MyPort[i].z][MyPort[i].x][MyPort[i].y])
					{
						Hinder.a[MyPort[i].z][MyPort[i].x][MyPort[i].y]++;
						cnt--;
					}
				}
			}
		}
		else
		{
			//���Ƚ����䴬��������
			for(i=0;My_Element_Now[CARGO]<Element_Max[CARGO] && Metal_Rest>=kProperty[CARGO].cost;i++)
			{
				Produce(CARGO);
				My_Element_Now[CARGO]++;
				Metal_Rest-=kProperty[CARGO].cost;
			}
			//����ת��������������
			{
				int cnt=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==FIGHTER && Tactics[INFO->elements[i]->index]==0)cnt++;
				if(cnt>=Element_Max[FIGHTER])//17�ɻ����ɽ�������
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==FIGHTER)
						Tactics[INFO->elements[i]->index]=1;
			}

			//��񱬷ɻ�
			if(INFO->round>1)
			while(My_Element_Now[FIGHTER]<Element_Max[FIGHTER]*2  && Metal_Rest>=kProperty[FIGHTER].cost)
			{
				Produce(FIGHTER);
				My_Element_Now[FIGHTER]++;
				Metal_Rest-=kProperty[FIGHTER].cost;
			}
			//�ڷɻ���������ʱ����ڷɻ���������ʱ�����ո�
			{
				int k=0;
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==FIGHTER)k++;
				int cnt=0;
				for(int i=0;i<INFO->production_num;i++)
				if(INFO->production_list[i].unit_type==FIGHTER)
					if(INFO->production_list[i].round_left<2)cnt++;
				cnt=min(cnt,Fuel_Rest/kProperty[FIGHTER].fuel_max);
				cnt=min(cnt,Element_Max[FIGHTER]-k);
				for(int i=0;i<INFO->element_num;i++)
					if(INFO->elements[i]->team==MyTeam && INFO->elements[i]->type==FIGHTER && INFO->elements[i]->ammo<kProperty[FIGHTER].ammo_max && distance(INFO->elements[i],MyBase)<=kProperty[FIGHTER].speed)
						cnt++;
				for(int i=0;cnt&&i<(int)MyAirport.size();i++)
				{
					for(k=0;k<INFO->element_num;k++)
						if(INFO->elements[k]->team == MyTeam && distance(INFO->elements[k]->pos,MyAirport[i])==0)
							break;
					if(k<INFO->element_num || !Hinder_Now.a[MyAirport[i].z][MyAirport[i].x][MyAirport[i].y])
					{
						Hinder.a[MyAirport[i].z][MyAirport[i].x][MyAirport[i].y]++;
						cnt--;
					}
				}
			}
		}
	}
	void Base_AI(const State *p)//����AI
	{
		//����ά������λ
		//���ȹ����з�������λ
		//���Ȳ�������λ

		//����
		if(!Attack.check(p))
		{
			Supply_Manager::Demand t=Demand.Supply_Dying(p);
			if(t.F!=-1)Supply(p->index,t.p->index,t.F,t.A,t.M);
			else
			{
				return;
			}
		}
	}
	void Fort_AI(const State *p)//�ݵ�AI
	{
		//���ȹ����з�������λ
		//���Ȳ�������λ

		//����
		if(!Attack.check(p))
		{
			Supply_Manager::Demand t=Demand.Supply_Dying(p);
			if(t.F!=-1)Supply(p->index,t.p->index,t.F,t.A,t.M);
			else
			{
				return;
			}
		}
	}
	bool Cargo_Collect(const State *p,int Type)//ǰ���غ������ɼ�Type����Դ�������Ƿ�ɹ�
	{
		const Property *P=&kProperty[p->type];
		const State *q;
		int k,t;
		int MinRest=INF,MinDis=INF;
		coor Order;
		//�ɼ�
		for(t=0;t<INFO->element_num;t++)
		{
			q=INFO->elements[t];
			if(q->type == Type && Resource.a[q->index]>0)
			{
				for(k=0;k<4;k++)
				{
					coor S;
					S.x = q->pos.x + Move_Offset[k][0];
					S.y = q->pos.y + Move_Offset[k][1];
					S.z = 1;
					if(check(S)&&Map(S.x,S.y)==OCEAN)
					{
						int dist=MyPortDist.b[S.x][S.y];
						int rest=(distance(p->pos,S) + P->speed - 1)/(P->speed) + (MyPortDist.b[S.x][S.y] + P->speed - 1)/(P->speed) + Cargo_Appointment.b[S.x][S.y]*3/2;
						if(rest<MinRest || (rest==MinRest && dist<MinDis))
						{
							MinRest=rest;
							MinDis=dist;
							Order=S;
						}
					}
				}
			}
		}
		if(MinRest<INF)
		{
			Cargo_Appointment.b[Order.x][Order.y]++;
			Move.Move_Straight(p,Resource.SPS[Resource.id[Order]],false);
			return true;
		}
		return false;
	}
	void Cargo_AI(const State *p)
	{
		//����ͼû����Դʱ��ɱ
		//����������Χ��λ�Ĳ�������
		//�����Ч�ʲɼ�(�����������������ɼ�ʱ��)
		//װ�ر���ʱ���ػ���
		//�ڿ����Χ�Ŀ����Ȳ����ڽ��Ĵ��ɿ��
		//�ڻ�����Χ�Ŀ������ظ���������ȴ�ֱ����Դ�ɹ�����
		
		//���Ͳ�������
		if(p->fuel==0 || (p->fuel<LowerFuel[p->type]&&DisToBase(p->pos,MyBase_Coor)<=1))
		{
			int require=(DisToBase(p->pos,MyBase_Coor)<=1)?LowerFuel[p->type]-p->fuel:Base_Attack_Range.b[p->pos.x][p->pos.y]+10;
			Demand.add(p,require);
		}

		//����Դ��������
		Supply_Manager::Demand t=Demand.Supply_Dying(p);
		if(t.F!=-1)Supply(p->index,t.p->index,t.F,t.A,t.M);
		else if(DisToBase(p->pos,MyBase_Coor)<=1)
		{
			//��������
			Supply(p->index,MyBase_Index,max(0,p->fuel - LowerFuel[p->type]),0,p->metal);
		}
		else
		{
			int i;
			for(i=0;i<INFO->element_num;i++)
			{
				const State *q=INFO->elements[i];
				if(q->type==2||q->type==3)
					if(distance(p,q)<=1)break;
			}
			//���ڿ����Χ���п����ѡ����������Ŀ󳵲���
			if(i<INFO->element_num)
			{
				for(i=0;i<INFO->element_num;i++)
				{
					const State *q=INFO->elements[i];
					if(q->team==MyTeam&&q->type==7&&p->index!=q->index&&distance(p,q)<=1)
					{
						Supply(p->index,q->index,max(0,p->fuel - LowerFuel[p->type]),0,p->metal);
						break;
					}
				}
			}
		}

		//�ɼ�
			//��������ʱ�ɼ�ʯ��
			//û�н���ʱ���Ȳɼ�����
			//����ʱ���ػ���
			bool Move_Success=false;//���ȵĲ����Ƿ�ɹ�
			if(Move_Success==false && p->fuel<=LowerFuel[p->type])Move_Success|=Cargo_Collect(p,3);//����
			if(Move_Success==false && p->metal==0)Move_Success|=Cargo_Collect(p,2);//�ɿ�
			//���ػ���
			if(Move_Success==false && (p->metal>0 || p->fuel>LowerFuel[p->type]))
			{
				Move.Move_Straight(p,MyPortDist);
				Move_Success=true;
			}
			if(Move_Success==false)
			{
				Move.Move_Straight(p,EnemyPortDist);
				Move_Success=true;
			}
		//�ɼ�END

	}
	void Carrier_AI(const State *p)//��ĸAI
	{
		//���������������ȼ��ѡ���Ƿ��ڻ�����Χͣ��
		//�ڻ�����Χȼ�ϲ���ʱ���ں��ⵯ������ʱ���Ͳ�������
		//����������Χ��λ�Ĳ�������
		//���Χ�з����صķ����ƶ�,�ƶ�������ѡȡ����ϡ���
		//��������ʱ���ؼ�������
		//������Χ�ڱ�����λ
		const Property *P=&kProperty[p->type];


		//���Ͳ�������
		if(p->fuel==0 || (p->fuel<LowerFuel[p->type]&&DisToBase(p->pos,MyBase_Coor)<=1))
		{
			int require=(DisToBase(p->pos,MyBase_Coor)<=1)?LowerFuel[p->type]-p->fuel:Base_Attack_Range.b[p->pos.x][p->pos.y]+10;
			Demand.add(p,require);
		}

		if(DisToBase(p->pos,MyBase_Coor)>1 || p->fuel>=LowerFuel[p->type])
		{
			if(MyTactics == 2 || MyTactics == 5  || Tactics[p->index]==1)Move.Move_Safety(p,Base_Attack_Range);//����
			else if(Tactics[p->index]==0)Move.Move_Safety(p,StagingDist);//����
		}
		
		//�˻���Դ
		if(p->fuel>LowerFuel[p->type]&&DisToBase(p->pos,MyBase_Coor)<=1)
		{
			Supply(p->index,MyBase_Index,p->fuel-LowerFuel[p->type],0,0);
		}
		//����
		else
		Attack.check(p);
	}
	void Submarine_AI(const State *p)//ǱͧAI
	{
		//�ڻ�����Χȼ�ϲ���ʱ���ں��ⵯ������ʱ���Ͳ�������
		//���Χ�з��ۿڵķ����ƶ�
		//������Χ�ڱ�����λ
		const Property *P=&kProperty[p->type];

		//���Ͳ�������
		if(p->fuel==0 || (p->fuel<LowerFuel[p->type]&&DisToBase(p->pos,MyBase_Coor)<=1))
		{
			int require=(DisToBase(p->pos,MyBase_Coor)<=1)?LowerFuel[p->type]-p->fuel:Base_Attack_Range.b[p->pos.x][p->pos.y];
			Demand.add(p,require);
		}


		if(DisToBase(p->pos,MyBase_Coor)>1 || p->fuel>=LowerFuel[p->type])
		{
			if(MyTactics == 2 || Tactics[p->index]==1)Move.Move_Straight(p,EnemyPortDist);//����
			else if(Tactics[p->index]==0)Move.Move_Straight(p,StagingDist);//����
		}

		//����
		Attack.check(p);
	}
	void Fighter_AI(const State *p)//����һ��ֱ�ƻ���
	{
		const Property *P=&kProperty[p->type];
		//if(DisToBase(p->pos,EnemyBase)<=P->fire_ranges[LAND])AttackUnit(p->index,EnemyBase_Index);

		Attack.check(p);
		if(p->ammo<kProperty[p->type].ammo_once)//��ҩ����ʱ�ڵо������ܱ��Ҷ�ֱ��ȼ�Ϻľ���׹��
		{
			if(distance(p->pos,EnemyBase_Coor)<P->speed-3)
			{
				for(int i=0;i<INFO->x_max;i++)
					for(int j=0;j<INFO->y_max;j++)
						MyAirportDist.b[i][j]*=-1;
				Move.Move_Straight(p,MyAirportDist);
				for(int i=0;i<INFO->x_max;i++)
					for(int j=0;j<INFO->y_max;j++)
						MyAirportDist.b[i][j]*=-1;
			}
			else
			Move.Move_Straight(p,MyAirportDist);//��ҩ���㷵�ػ���
		}
		else
		if(MyTactics==0 && Tactics[p->index]==0)//��������
		{
			Move.Move_Straight(p,MyAirportDist);
		}
		else 
		if(MyTactics==1 || Tactics[p->index]==1)//����
		{
			if(distance(p,EnemyBase)>1)Move.Move_Straight(p,EnemyAirportDist);
			else
			{
				//������ռ�����ϿյĿ�λ
				if(distance(p,EnemyBase)==0)Hinder.a[p->pos.z][p->pos.x][p->pos.y]++;
				else
				{
					int i,j,k=0x7fffffff;
					coor order;
					for(i=0;i<(int)EnemyAirport.size();i++)
					{
						coor T=EnemyAirport[i];
						j=Hinder.a[T.z][T.x][T.y]*1000-distance(p->pos,T);
						if(j<k)
						{
							order=T;
							k=j;
						}
					}
					Position Target;
					Target.x=order.x;
					Target.y=order.y;
					Target.z=order.z;
					ChangeDest(p->index,Target);
				}
			}
		}
	}
	void Scout_AI(const State *p)
	{
		const Property *P=&kProperty[p->type];

		//���Ͳ�������
		if(p->fuel==0 || (p->fuel<LowerFuel[p->type]&&DisToBase(p->pos,MyBase_Coor)<=1))
		{
			int require=(DisToBase(p->pos,MyBase_Coor)<=1)?LowerFuel[p->type]-p->fuel:Base_Attack_Range.b[p->pos.x][p->pos.y]+10;
			Demand.add(p,require);
		}

		//����
		Attack.check(p);

		//�����з����䴬/����/�ݵ� ����λ�����괦�ƶ�
		vector<int>x_,y_;
		for(int i=0;i<INFO->element_num;i++)
			if(INFO->elements[i]->type>3 && INFO->elements[i]->type!=CARGO && INFO->elements[i]->type!=SCOUT)
			{
				const State *q=INFO->elements[i];
				x_.push_back(q->pos.x);
				y_.push_back(q->pos.y);
			}
		coor target=p->pos;
		if(x_.size())
		{
			sort(x_.begin(),x_.end());
			sort(y_.begin(),y_.end());
			int x=x_[x_.size()/2],y=y_[y_.size()/2];
			
			if(distance(coor(x,y),p->pos)>3)target=coor(x,y,2);
		}
		Shortest_Path_Struct SPC;
		SPC.Shortest_Path(target);
		Move.Move_Safety(p,SPC);
	}
}AI;
const int DEBUG=true;
const char Element_Name[10][10]={"����0","�ݵ�1","��2","����3","Ǳͧ4","����5","��ĸ6","���佢7","ս����8","����9"};
/*struct DEBUGER_
{
	void draw(char *s){if(!DEBUG)return;string S(s);output<<S;}
	void draw(coor a){if(!DEBUG)return;output<<'('<<a.x<<','<<a.y<<','<<a.z<<')';}
	void draw(Size a){if(!DEBUG)return;output<<'('<<a.x_length<<','<<a.y_length<<')';}
	void draw(State a)
	{
		if(!DEBUG)return;
		char order[500];
		sprintf_s(order,"Index=%d %s:(%d,%d,%d) Team=%d Visible=%d Health=%d Fuel=%d Ammo=%d Metal=%d Destination:(%d,%d,%d)\n",a.index,Element_Name[a.type],a.pos.x,a.pos.y,a.pos.z,a.team,a.visible,a.health,a.fuel,a.ammo,a.metal,a.destination.x,a.destination.y,a.destination.z);
		draw(order);
	};
	void draw(ProductionEntry a)
	{
		if(!DEBUG)return;
		char order[30];
		sprintf_s(order,"(%s: %d rd)",Element_Name[a.unit_type],a.round_left);
		draw(order);
}
	void draw(const GameInfo &a)
	{
		if(!DEBUG)return;
		char order[500];
		if(a.round==0)
		{
			sprintf_s(order,"MyTeam=%d MapSize(%d,%d)  Population_Limit=%d  Round_Limit=%d Weather=%d Time_Per_Round=%.2f\n",a.team_num,a.x_max,a.y_max,a.population_limit,a.round_limit,a.weather,a.time_per_round);
			draw(order);
			for(int i=0;i<INFO->x_max;i++,output<<endl)
				for(int j=0;j<INFO->y_max;j++)output<<Map(i,j)<<' ';
			output<<"\n\n\n\n\n";
		}
		sprintf_s(order,"Round:%d    Scores:(%d,%d)    Population=%d Element_Num=%d Production_Num=%d\n",a.round,a.scores[0],a.scores[1],a.population,a.element_num,a.production_num);
		draw(order);
		for(int k=0;k<3;k++)
			for(int j=0;j<10;j++)
				for(int i=0;i<a.element_num;i++)
					if(a.elements[i]->team==k&&a.elements[i]->type==j)
						draw(*a.elements[i]);
		for(int i=0;i<a.production_num;i++)draw(a.production_list[i]);
		output<<"\n\n\n\n\n\n"<<endl;
	}
}DEBUGER;*/
void AIMain()
{  
	TryUpdate();
	//DEBUGER.draw(*INFO);
	//�غϿ�ʼʱ�ĳ�ʼ��
	if(!AI_Begin)
	{
		AI.AI_Start();
		AI_Begin=true;
	}
	//ֻ�������������ƶ�
	/*{
		Attack.clear();
		Attack.init();
		for(int i=0;i<INFO->element_num;i++)
			if(INFO->elements[i]->team==MyTeam)
				Attack.check(INFO->elements[i]);
		Attack.solve();
		return;
	}*/

	AI.AI_Run();
}
