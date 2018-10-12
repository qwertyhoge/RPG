/*
1.作った点
・自力の非カノニカルモードの実装
・マップのランダム生成
・スクロール(自機中心)
・戦闘，敵の追加
・レベルアップと成長
・視界(壁考慮せず，正方形)
・武器，防具の追加(多分まともに機能しない,アイテムに関しては中身が未実装)
・アイテムIDのようなものを作っているのに全く使っていない
・階層を作った 25階で終わりにしたいが未実装

2.圧倒的スパゲッティコード
3.関数機能確認できず->アイテム画面で存在しないアイテムを押すとsegmentation fault
4.マップがひどい
5.
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>

#define dista(tile1,tile2) ((tile1)-(tile2))

#define CLEARSCREEN(i) for(i=1;i<=30;i++)puts("")

//マップ表示距離
#define VIEWX 20
#define VIEWY 10

//マップ最大範囲
#define MAXX 70
#define MAXY 45

//マップ生成のブロックの最大/最小の半径の長さ
#define BLOCKMAX 8
#define BLOCKMIN 3

//視界距離
#define SIGHTRANGE 4

//デバッグ用定数
#define DEBUG 0

int map[MAXY][MAXX];
int clear[MAXY][MAXX];
char symbol[MAXY][MAXX];
int maxx,maxy;
int stairx,stairy;
int player_x,player_y;
int maxhp=10;
int lv=1,hp=10,atk=3,def=0,spd=0,gold=0,expe=0;
int weap=0,guard=0;
int havefire=0,havetusk=0;
int bat=0,d=0,red=0,inj=0,reinj=0,down=0,rec=0,l=0,g=0,ex=0;
int Floor=0;
int clsc;

int makemap(int,int);
void showmap(int);
char enemy(void);
void info(int);
int encount(int,int);
int lvup(void);
int items(int);
int sight(int,int,int*);
int battle(int*,char*);
char receptor(void);
void debug(void);


typedef struct {
  char Symbol;
  char Name[15];
  int Hp;
  int Atk;
  int Def;
  int Spd;
  int Exp;
  int Gold;
  int Item;
}enemy_t;

typedef struct {
  char name[15];
  int pow;
}weapon_t;

typedef struct {
  char name[15];
  int guards;
}guard_t;

typedef struct {
  char name[15];
  int equip;
  char expl[30];
  int number;
}item_t;

struct debug_t {

  int blockvol;
  int blockcenterx[10];
  int blockcentery[10];
  int pathway[10];
  int pathp[10];
  int connect[10];
  int receptway[10];
  int receptp[10];
  int pre_pathx[10];
  int pre_pathy[10];
};

const enemy_t enstat[]= {//SymbNameHpAtkDefSpdExpGoldItem
  {'s',"Slime",10,3,1,0,3,1,1},
  {'b',"Bat",6,5,0,3,3,2,2},
  {'t',"Troll",20,10,0,-2,7,4,3},
  {'z',"Zonbie",40,10,0,0,10,4,4},
  {'G',"Giant",60,15,0,0,15,7,5},
  {'F',"FlameElemental",20,30,30,0,18,5,6},
  {'V',"Vampire",40,20,2,8,40,25,7},
  {'K',"Knight",60,30,10,3,50,30,8},
  {'D',"Dragon",150,60,10,5,60,40,9},
  {'N',"Nyarlathotep",500,50,10,8,0,0,10}
  
};


const weapon_t weapstat[]= {
  {"空手",0},
  {"棍棒",4},
  {"Giantの拳",6},
  {"ナイトソード",10},
  {"ドラゴンの剣",20}


};

const guard_t gdstat[]= {
  {"服",0},
  {"Trollの服",3},
  {"Giantの服",5},
  {"ナイトアーマー",10},
  {"ドラゴンの鎧",15}

};

const item_t itemstat[]= {
  {"棍棒",1,"Trollの武器.",101},
  {"Giantの拳",1,"文字通りの大きな拳.",102},
  {"ナイトソード",1,"正式な鉄の剣.",103},
  {"ドラゴンの剣",1,"ドラゴンの牙で作られた剣.",104},
  {"Trollの服",1,"頑丈な服.",201},
  {"Giantの服",1,"大きく頑丈な服.",202},
  {"ナイトアーマー",1,"正式な鉄の鎧.",203},
  {"ドラゴンの鎧",1,"ドラゴンの鱗で作られた鎧.",204},
  {"血塗れの牙",0,1},
  {"生きている炎",0,2}


};

struct debug_t pathtest;

 
main(void)
{
  srandom(time(NULL));

  int x, y;
  int i,j;
  int fl=1;
  int px,py;
  char direction;
  int direc;
  int error=0;


  puts("テスト文章\n");
  puts("アイコンについて:");
  puts(" ,  食料     体力を回復する．");
  puts(" $  硬貨       金．");
  puts(" >  階段     迷宮のより深くに降りる階段．");
  printf("\n何かキーを押してください...");
  receptor();

  puts("戦闘について:");
  puts("");


  printf("\n");

  while (hp>0){

    /*マップ生成部分*/
    if(fl==1){
      maxx=MAXX-rand()%30;
      maxy=MAXY-rand()%20;
      Floor++;

      for(i=0;i<MAXY;i++)
	for(j=0;j<MAXX;j++)
	  clear[i][j]=0;
    
      makemap(maxx,maxy);
    }


     
    CLEARSCREEN(clsc);


    showmap(0);
 
    
    info(fl);//状況変化の時，対応する変数を1などにしてマップ表示の後に遅らせて一括表示する
    bat=0;
    l=0;
    d=0;
    red=0;
    inj=0;
    reinj=0;
    down=0;
    rec=0;
    g=0;
    ex=0;
    fl=0;
    d=0;

    printf("***レベル:%d 体力:%d/%d お金:%d 経験値:%d 現在の階層:%d***\n",lv,hp,maxhp,gold,expe,Floor);
    printf("player_x=%d,player_y=%d\n",player_x,player_y);
    
    /* プレイヤの移動*/
    
    printf("> ");

    direction=receptor();
    
    switch(direction){
    case '\n':
      menu();
      break;
    case '1':
      px = player_x-1;
      py = player_y+1;
      break;
    case '2':
      px = player_x;
      py = player_y+1;
      break;
    case '3':
      px = player_x+1;
      py = player_y+1;
      break;
    case '4':
      px = player_x-1;
      py = player_y;
      break;
    case '6':
      px = player_x+1;
      py = player_y;
      break;
    case '7':
      px = player_x-1;
      py = player_y-1;
      break;
    case '8':
      px = player_x;
      py = player_y-1;
      break;
    case '9':
      px = player_x+1;
      py = player_y-1;
      break;
    default:
      px=player_x;
      py=player_y;
      break;
    }
    
  
    error=0;
    if(map[py][px]!=4 && map[py][px]!=1){
      player_x=px;
      player_y=py;

    }else if(map[py][px]==4){
      d=battle(&map[py][px],&symbol[py][px]);
      hp-=d;
      if(map[py][px]==0){
	player_x=px;
	player_y=py;
      }

    }
      
    
    
    if (map[player_y][player_x] == 2){
      /* 体力回復 */
      rec = random() % 3+5;
      hp += rec;

      if(hp>=maxhp)
	hp=maxhp;

      map[player_y][player_x] = 0;

    }else if (map[player_y][player_x] == 3){
      /* 所持金増加 */
      g = random() % 3+5;
      gold = gold + g;
      map[player_y][player_x] = 0;

    }else if(map[player_y][player_x]==5){
      fl=1;
    }
    
  }
  printf("あなたは探索の果て，力尽きた．\n");
  receptor();

}
/*end of main*/







/*マップ作成関数 多分一番複雑で一番スパゲッティコード*/
int makemap(int setx,int sety)
{

  int i,j,k,f;
  int x,y;
  int centx,centy;
  int blocksizex,blocksizey; //半径
  int path[10];
  int recept;
  int pathpoint;
  int receptpoint;
  int pathx[10];
  int pathy[10];
  int receptx[10];
  int recepty[10];
  int decide=0;
  int blocks=0,fail=0;
  int dx,dy,distance[10][10],shortest[10];
  int connects=0;
  int close[10];

  //まず全マスを壁にする
  for(i=0;i<sety;i++){
    for(j=0;j<setx;j++){
      map[i][j]=1;
    }
  }

  while((blocks<3||fail<50)&&fail<200){

    //生成するブロック(部屋)の中央を設定 建てれないものはあとではじく
    centx=rand()%maxx;
    centy=rand()%maxy;

#if DEBUG == 1
    pathtest.blockcenterx[blocks]=centx;
    pathtest.blockcentery[blocks]=centy;
#endif

    blocksizex=rand()%(BLOCKMAX-BLOCKMIN)+1+BLOCKMIN;
    blocksizey=rand()%(BLOCKMAX-BLOCKMIN)+1+BLOCKMIN;
    
    //2マスの余裕を持たせて部屋生成範囲に床がないか、端でないか調べる
    for(i=(centy-blocksizey-2);i<=(centy+blocksizey+2);i++){
      for(j=(centx-blocksizex-2);j<=(centx+blocksizex+2);j++){
	if(i<=0 || i>=sety-1 || j<=0 || j>=setx-1){
	  decide=0;
	  break;
	  
	}
	if(map[i][j]==0){
	  decide=0;
	  break;
	}else  decide=1;
	
      }
      if(i<=0 || i>=sety-1 || j<=0 || j>=setx-1){
	fail++;
	decide=0;
	break;
	
      }
      if(map[i][j]==0){
	fail++;
	decide=0;
	break;
      }
	
      j=0;
    }
    

    if(decide==1){

      //実際にブロックを生成
      for(i=(centy-blocksizey);i<=(centy+blocksizey);i++){
	for(j=(centx-blocksizex);j<=(centx+blocksizex);j++){
	  map[i][j]=0;
	}
	j=0;
      }

      //部屋をつなぐ通路の予約,2マス実際に生やしておく
      path[blocks]=rand()%4; //通路の方向

#if DEBUG == 1
      pathtest.pathway[blocks]=path[blocks];
#endif
      if(path[blocks]==0||path[blocks]==2){//通路の場所が上/下辺
	pathpoint=rand()%(blocksizex*2) + centx-blocksizex;
	if(path[blocks]==0){//上
	  map[centy-blocksizey-1][pathpoint]=0;
	  map[centy-blocksizey-2][pathpoint]=0;
	  pathx[blocks]=pathpoint;
	  pathy[blocks]=centy-blocksizey-2;
#if DEBUG == 1
	  pathtest.pre_pathx[blocks]=pathpoint;
	  pathtest.pre_pathy[blocks]=centy-blocksizey-2;
#endif
	}else if(path[blocks]==2){//下
	  map[centy+blocksizey+1][pathpoint]=0;
	  map[centy+blocksizey+2][pathpoint]=0;
	  pathx[blocks]=pathpoint;
	  pathy[blocks]=centy+blocksizey+2;
#if DEBUG == 1
	  pathtest.pre_pathx[blocks]=pathpoint;
	  pathtest.pre_pathy[blocks]=centy+blocksizey+2;
#endif
	}

      }else if(path[blocks]==1||path[blocks]==3){//通路の場所が右/左辺
	pathpoint=rand()%(blocksizey*2) + centy-blocksizey;
	if(path[blocks]==1){//左
	  map[pathpoint][centx-blocksizex-1]=0;
	  map[pathpoint][centx-blocksizex-2]=0;
	  pathx[blocks]=centx-blocksizex-2;
	  pathy[blocks]=pathpoint;
#if DEBUG == 1
	  pathtest.pre_pathx[blocks]=centx+blocksizex+1;
	  pathtest.pre_pathy[blocks]=pathpoint;
#endif
	}else if(path[blocks]==3){//右
	  map[pathpoint][centx+blocksizex+1]=0;
	  map[pathpoint][centx+blocksizex+2]=0;
	  pathx[blocks]=centx+blocksizex+2;
	  pathy[blocks]=pathpoint;
#if DEBUG == 1
	  pathtest.pre_pathx[blocks]=centy+blocksizey+1;
	  pathtest.pre_pathy[blocks]=pathpoint;
#endif
	}
       
      }
#if DEBUG == 1
      pathtest.pathp[blocks]=pathpoint;
#endif
 
    
      //伸びてくる通路を受ける場所を設定
      recept=rand()%4;
#if DEBUG == 1
      pathtest.receptway[blocks]=recept;
#endif
      if(recept==0||recept==2){//受ける場所が上/下辺
	do{
	  receptpoint=rand()%(blocksizex*2) + centx-blocksizex;
	}while(pathpoint==receptpoint);
	if(recept==0){//上
	  receptx[blocks]=receptpoint;
	  recepty[blocks]=centy-blocksizey;
	}else if(recept==2){//下
	  receptx[blocks]=receptpoint;
	  recepty[blocks]=centy+blocksizey;
	}

      }else if(recept==1||recept==3){//受ける場所が右/左辺
	do{
	  receptpoint=rand()%(blocksizey*2) + centy-blocksizey;
#if DEBUG == 1
	  pathtest.receptp[blocks]=receptpoint;
#endif
	}while(pathpoint==receptpoint);
	if(recept==1){//左
	  receptx[blocks]=centx-blocksizex;
	  recepty[blocks]=receptpoint;
	}else if(recept==3){//右
	  receptx[blocks]=centx+blocksizex;
	  recepty[blocks]=receptpoint;
	}
      }
      
      fail=0;
      blocks++;

    }
    
    
  }

  
  //以下通路の実際の生成,一つ目の部屋の通路を起点に基準点を変えて部屋をつなぐ.

  for(i=0;i<blocks;i++){
    close[i]=0;
    shortest[i]=0;
  }



  pathtest.blockvol=blocks;

#if DEBUG == 1
  printf("blocks=%d\n\n",blocks);
#endif

  i=0;
  close[0]=1;
  do{
    
    for(j=0;j<blocks;j++){
#if DEBUG == 1
      printf("in %d回目 \"J\"loop of before make paths\n",j);
#endif
      //x,yそれぞれの距離を測る
      dx=pathx[i]-receptx[j];
      if(dx<0)
	dx=0-dx;
      dy=pathy[i]-recepty[j];
      if(dy<0)
	dy=0-dy;
      
      //距離を記録
      distance[i][j]=dx+dy;
#if DEBUG == 1
      printf("distance[%d][%d]=%d\n",i,j,distance[i][j]);
#endif
      
      if(connects+1==blocks){
	shortest[i]=0;
#if DEBUG == 1
	printf("last loop,set shortest 0.\n");
#endif
      }else{

	if(j>0){//初期化の必要なし
	  if(i!=j&&close[j]==0){//自己完結せず開いている
	    if(distance[i][j]<distance[i][j-1]){//前回より短いか?
	      shortest[i]=j;
#if DEBUG == 1
	      printf("shortest[%d]=%d\n",i,shortest[i]);
#endif
	      
	    }
	  }
	  
	}else{//初期化が必要であれば(=iが0)
	  k=0;
	  while(close[j+k]==1||i==j+k)//つなげるところが開いて自己完結しないまで見る
	    k++;
	  if(j+k>=blocks){
	    while(close[j+k]==1||i==j+k)
	      k--;
	  }
	  shortest[i]=j+k;
#if DEBUG == 1
	  printf("shortest[%d]=%d\n",i,shortest[i]);
#endif
	}

      }
      
#if DEBUG == 1
      if(j==blocks-1){
	printf("\nlatest shortest[%d]=%d\n",i,shortest[i]);
	pathtest.connect[i]=shortest[i];
      }
#endif
      
    }

#if DEBUG == 1
    receptor();
#endif

    if(path[i]==0||path[i]==2){//通路が部屋の上下辺にある時の処理

      /*
	横に通路を伸ばす処理,for文の変数の都合で通路と通路を受ける場所のどちらが
	左右にあるか比べてから通路を伸ばす
	こちらは通路を伸ばす側を主点にしている
       */

      if(pathx[i]>=receptx[shortest[i]]){//受け取りが右側,kは右に増える
	for(k=receptx[shortest[i]];k<=pathx[i];k++){
	  map[pathy[i]][k]=0;
	}
      }else if(pathx[i]<receptx[shortest[i]]){//受け取りが左側,kは左に増える
	for(k=receptx[shortest[i]];k>=pathx[i];k--){
	  map[pathy[i]][k]=0;
	}
      }
      /*
	縦に通路を伸ばす形で受ける処理,これもfor文の変数の都合で上下を比べる
	こちらは通路を受け取る側を主点にしている
       */

      if(recepty[shortest[i]]<=pathy[i]){//受け取りが上側,kは下に増える
	for(k=recepty[shortest[i]];k<=pathy[i];k++){
	  map[k][receptx[i]]=0;

	}

      }else if(recepty[shortest[i]]>pathy[i]){//受け取りが下側,kは上に増える
	for(k=recepty[shortest[i]];k>=pathy[i];k--){
	  map[k][receptx[i]]=0;
	  
	}
      }

    }else if(path[i]==1||path[i]==3){//通路が部屋の左右辺にある場合の処理

      /* 縦に通路を伸ばす */
      
      if(pathy[i]>=recepty[shortest[i]]){//受け取りが下側,kは下に増える
	for(k=recepty[shortest[i]];k<=pathy[i];k++){
	  map[k][pathx[i]]=0;
	}
      }else if(pathy[i]<recepty[shortest[i]]){//受け取りが上側,kは上に増える
	for(k=recepty[shortest[i]];k>=pathy[i];k--){
	  map[k][pathx[i]]=0;
	}
      }

      /* 横に受け取り側から通路を伸ばして受ける */


      if(receptx[shortest[i]]<=pathx[i]){//受け取りが左側,kは右に増える
	for(k=receptx[shortest[i]];k<=pathx[i];k++){
	  map[recepty[i]][k]=0;
	}
      }else if(receptx[shortest[i]]>=pathx[i]){//受け取りが右側,kは左に増える
	for(k=receptx[shortest[i]];k>=pathx[i];k--){
	  map[recepty[i]][k]=0;
	}

      }

    }
    close[shortest[i]]=1;
    connects++;
    j=0;
    i=shortest[i];
  }while(connects<blocks);

  
  for(i=1;i<sety-1;i++){
    for(j=1;j<setx-1;j++){
      if(map[i][j]==0){
	switch(rand()%10){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	  map[i][j]=0; //床
	  break;
#if DEBUG == 0
	case 6:
	  for(k=i-1;k<=i+1;k++){
	    for(f=j-1;f<=j+1;f++){
	      if(map[k][f]==1){
		decide=0;
		break;
	      }else decide=1;

	    }
	    if(map[k][f]==1)
	      break;
	    f=j-1;
	  }

	  if(decide==1)
	    map[k][f]=1; //壁
	  break;
#endif
	case 7:
	  map[i][j]=2; //食料
	  break;
	case 8:
	  map[i][j]=3; //金
	  break;
	case 9:
	  map[i][j]=4; //敵
	  symbol[i][j]=enemy();
	  break;
	default:
	  break;
	}
      }
    }
  }
    
  decide=0;

  do{
    player_x=rand()%maxx;
    player_y=rand()%maxy;

    if(player_y!=0 && player_y!=maxy-2
       && player_x!=0 && player_x!=maxx-2
       && map[player_y][player_x]!=1)
      decide=1;

    

  }while(decide!=1);

  decide=0;
  do{
    stairx=rand()%maxx;
    stairy=rand()%maxy;

    if(stairy!=0 && stairy!=maxy-2
       && stairx!=0 && stairx!=maxx-2
       && !(stairx==player_x && stairy==player_y)
       && map[stairy][stairx]!=1)
      decide=1;

  }while(decide!=1);

  map[stairy][stairx]=5;

}
      
      
      
      
      
      

void showmap(int all)
{
  int x,y;

  /*  MAP表示  */
  for(y=0;y<maxy;y++){
    for(x=0;x<maxx;x++){
      if(player_y-VIEWY <y && y<player_y+VIEWY){
	if((player_x-VIEWX) <x && x< player_x+VIEWX){
	  if (x == player_x && y == player_y){
	    printf("@");
	  }else{
#if DEBUG == 0
	    if(sight(x,y,&clear[y][x])==1 || clear[y][x]==1){
#endif
	      if (map[y][x] == 0){
		printf(".");
	      }else if (map[y][x] == 1){
		printf("#");
	      }else if (map[y][x] == 2){
		printf(",");
	      }else if (map[y][x] == 3){
		printf("$");
	      }else if (map[y][x] == 4){
		printf("%c",symbol[y][x]);
	      }else if (map[y][x] == 5){
		printf(">");
	      }
#if DEBUG == 0
	    }else printf(" ");
#endif
	  }
	  
	}
	
      }
      if(all!=1){
	if(x>player_x+VIEWX)
	  break;
      }
    }
    if(all!=1){
      if(y>player_y+VIEWY)
	break;
    }
    x=0;
    puts("");
  }
  
  
}
 




 
/*ログらしきもの*/
void info(int deeper)
{

  if(bat==1){
    if(inj!=0)
      printf("あなたは敵に%dのダメージを与え",inj);
    if(reinj!=0)
      printf("，さらに%dのダメージを与え",reinj);
    if(down!=0){
      printf("て敵を倒し，");
      printf("%dの金,%dの経験値を得",g,ex);
      if(l!=0){
	printf("て，レベルが%d上がった.",l);
      }else puts("た．");
    }else puts("た．");
  
    if(d!=0)
      printf("あなたは敵から%dのダメージを受け",d);
    if(red!=0)
      printf("，さらにあなたは敵から%dのダメージを受け",red);
    if(hp<=0){
      printf("，あなたは力尽きた.");
      receptor();
      exit(0);
    }else puts("た.");

  }

  if(rec!=0){
    printf("あなたは%d回復した．",rec);
  }
  if(g!=0){
    printf("あなたは%dの金を拾った.",g);
  }
  if(deeper!=0){
    if(Floor!=1)
      puts("あなたは階段を降り，より深くに潜った．");
    printf("現在，あなたは%d階にいる．",Floor);
  }

#if DEBUG == 1

  debug();

#endif

  puts("");
}





  
int menu(void){
  
  char *choice[]={"cancel","useitem","seemap","seestatus"};
  char c;
  int ch;
  int x,y;
  int redo=0;

  do{

    CLEARSCREEN(clsc);
    
    printf("0.\t%s\n1.\t%s\n2.\t%s\n3.\t%s\n",choice[0],choice[1],choice[2],choice[3]);
    
    c=receptor();
    ch=c-'0';
    
    switch(ch){
    case 0:
    default:
      showmap(0);
      printf("> ");
      return 1;
      break;
      
    case 1:
      redo=items(0);
      if(redo==0)
	return 0;

      break;
      
    case 2:
      showmap(1);
      puts("\n何かキーを押してください．");
      receptor();
      redo=1;

    case 3:
      printf("装備:%s\t%s\nHP:%d\tATK:%d\nDEF:%d\tSPD:%d\n",weapstat[weap].name,gdstat[guard].name,hp,atk,def,spd);
      printf("何かキーを押してください．.");
      receptor();
	redo=1;
      
    }

  }while(redo!=0);
    

}






/*function of spawning enemy*/
char enemy(void)
{
  char choosensymb;
  int choose;

  choose=rand()%3;

  if(Floor>=2)
    choose++;
  if(Floor>=3)
    choose++;
  if(Floor>=4)
    choose++;
  if(Floor>=5)
    choose++;
  if(Floor>=6)
    choose++;
  if(Floor>=7)
    choose++;
  if(Floor==8)
    choose=9;

  choosensymb=enstat[choose].Symbol;

  return choosensymb;

}






int battle(int *kill,char *symbol)
{
  int enemy;
  int difspd;
  int dmg;
  int dmgd;
  int redmgd;
  int rem_hp;
  char symb;
  static int enhp;
  
  symb=*symbol;

  if(symb=='s'){
    enemy=0;
  }else if(symb=='b'){
    enemy=1;
  }else if(symb=='t'){
    enemy=2;
  }else if(symb=='z'){
    enemy=3;
  }else if(symb=='G'){
    enemy=4;
  }else if(symb=='F'){
    enemy=5;
  }else if(symb=='V'){
    enemy=6;
  }else if(symb=='K'){
    enemy=7;
  }else if(symb=='D'){
    enemy=8;
  }else if(symb=='N')//SymbNameHpAtkDefSpdExpGoldItem
    enemy=9;
  
  if(enhp==0)
    enhp=enstat[enemy].Hp;
  
  bat=1;

  //こちらのspdが早ければdifspdは正
  difspd=spd-enstat[enemy].Spd;
  if(difspd>=0){
    dmg = atk + rand()%3 - enstat[enemy].Def;
    enhp -= dmg;
    inj=dmg;
  
    if(enhp>0){
      if(rand()%10+1<difspd){
	dmg = atk + rand()%3 - enstat [enemy].Def;
	enhp -= dmg;
	reinj=dmg;
      }
    }
    if(enhp<=0){
      *kill=0;
      *symbol=0;
      down=1;
      
      ex=enstat[enemy].Exp;
      expe+=ex;
      g=enstat[enemy].Gold;
      gold+=g;
      l=lvup();
    }


  }

  dmgd=enstat[enemy].Atk+rand()%3 - def;
  rem_hp=hp-dmgd;

  d=dmgd;
  
  
  if(difspd<0){
    if(rem_hp>0){
      if(rand()%10<-difspd){
	redmgd = enstat[enemy].Atk+rand()%3 - def;
	dmgd+=redmgd;
	red=redmgd;
      }
    }
    
    rem_hp=hp-dmgd;
    
    if(rem_hp>0){
      dmg = atk + rand()%3 - enstat[enemy].Def;
      enhp -= dmg;
      
    }
    
    if(enhp<=0){
      *symbol=0;
      *kill=0;
      ex=enstat[enemy].Exp;
      expe+=ex;
      g=enstat[enemy].Gold;
      gold+=g;
      l=lvup();
      items(enstat[enemy].Item);
    }
    
  }


  return dmgd;
}





int items(int num)
{

  int i;
  char c;
  int ch;
  int chh;
  int error=0;
  int count=1;
  static int have[10];
  static int equip[10];


  if(num==0){//use
    
    do{

      CLEARSCREEN(clsc);

      printf("0.\tキャンセル\n");
      for(i=0;i<10;i++){

	if(have[i]==1)
	  printf("%d.\t%s\n",i+1,itemstat[i].name);
	
      }

      printf("どれを選びますか?\n> ");
      c=receptor();
      ch=c-'0'-1;

      switch(ch){
      case 0:
      default:
	return 1;
	break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
	if(have[ch]==1){
	  if(equip[ch]==0){
	    printf("1.\t装備する\n");
	    
	    
	  }else if(equip[ch]==1){
	    printf("1.\t外す\n");
	    
	    
	  }
	  printf("2.\t捨てる\n");
	  printf("0.\tキャンセル\n");
	  printf("選択\n> ");
	  do{
	    c=receptor();
	    chh=c-'0';
	    switch(chh){
	    case 0:
	      break;
	    case 1:
	      if(equip[ch]==0){
		
		if(ch>=0&&ch<=4){
		  for(i=0;i<=4;i++){
		    equip[i]=0;
		  }
		  equip[ch]=1;
		  atk+=weapstat[ch].pow;
		  weap=ch;
		}
		
		if(ch<4&&ch<=8){
		  for(i=5;i<=8;i++){
		    equip[i]=0;
		  }
		  equip[ch]=1;
		  def+=gdstat[ch].guards;
		  guard=ch;
		}
		
	      }else if(equip[ch]==1){
		equip[ch]=0;
		if(ch>=0&&ch<=4){
		  atk-=weapstat[ch].pow;
		  weap=0;
		}
		
		if(ch<4&&ch<=8){
		  def-=gdstat[ch].guards;
		  guard=0;
		}
		
	      }
	      
	      break;
	    case 2:
	      printf("本当に捨てますか？(y/n)");
	      c=receptor();
	      if(c=='y'){
		have[ch]=0;
	      }else printf("捨てるのをやめました．");
	      break;
	    default:
	      error=1;
	      break;
	      
	    }
	    
	  }while(error!=0);
	  error=0;
	}
	break;
      case 9:
      case 10:
	if(have[ch]==1){
	  printf("1.\t使う\n");
	  printf("2.\t捨てる\n");
	  printf("0.\tキャンセル\n");
	  do{
	    c=receptor();
	    chh=c-'0';
	    switch(chh){
	    case 1:
	      /*
		break;
	      */
	    case 0:
	    default:
	      break;
	    case 2:
	      printf("本当に捨てますか？(y/n)");
	      c=receptor();
	      if(c=='y'){
		have[ch]=0;
	      }else printf("捨てるのをやめました．");
	      break;
	      
	    }
	    
	  }while(error!=0);
	  error=0;
	 
	}

      }

    }while(error!=0);
      

  }else{
    if(num==3){//Trollのドロップ

      if(have[0]!=1){
	if(rand()%100==0){
	  have[0]=1;
	  printf("棍棒を手に入れた.");
	}
      }

      if(have[4]!=1){
	if(rand()%100==1){
	  have[4]=1;
	  printf("Trollの服を手に入れた.");
	}
      }


    }else if(num==5){//Giantのドロップ

      if(have[1]!=1){
	if(rand()%100==0){
	  have[1]=1;
	  printf("Giantの拳を手に入れた.");
	}
      }
      
      if(have[5]!=1){
	if(rand()%100==1){
	  have[5]=1;
	  printf("Giantの服を手に入れた.");
	}
      }
      
    }else if(num==6){//FlameElementalのドロップ

      if(rand()%100==0){
	havefire++;
	printf("生きている炎を手に入れた．");
      }

    }else if(num==7){//Vampireのドロップ

      if(rand()%100==0){
	havetusk++;
	printf("血塗れの牙を手に入れた.");
      }


    }else if(num==8){//Knightのドロップ

      if(have[2]!=1){
	if(rand()%100==0){
	  have[2]=1;
	  printf("ナイトソードを手に入れた.");
	}
      }

      if(have[6]!=1){
	if(rand()%100==1){
	  have[6]=1;
	  printf("ナイトアーマーを手に入れた.");
	}
      }

    }else if(num==9){//Dragonのドロップ

      if(have[3]!=1){
	if(rand()%100==0){
	  have[3]=1;
	  printf("ドラゴンの剣を手に入れた");
	}
      }
      if(have[8]!=1){
	if(rand()%100==0){
	  have[8]=1;
	  printf("ドラゴンの鎧を手に入れた");

	}
      }

    }
    
    
  }
  
  
}






int lvup(void)
{

  static int needexp=10;
  int lvlup=0;
  static int growhp=10;
  static int growatk=1;
  static int growdef=1;

  while(expe>=needexp){
    lvlup++;
    lv++;
    needexp+=lv*10;

    if(1<=lv&&lv<4){
      growhp+=15;
      growatk+=1;
      growdef+=1;
    }else if(4<=lv&&lv<7){
      growhp+=30;
      growatk+=2;
      growdef+=2;
    }else if(7<=lv&&lv<11){
      growhp+=50;
      growatk+=4;
      growdef+=4;
    }else if(11<=lv&&lv<15){
      growhp+=60;
      growatk+=4;
      growdef+=4;
    }

    if(lv%5==0)
      spd++;

    maxhp=growhp;
    atk=growatk;
    def=growdef;
    printf("\nneedexp:%d\n",needexp);
    printf("expe:%d\n",expe);
  }


  return lvlup;
}






/*視界の関数*/
int sight(int point_x,int point_y,int *clflag)
{
  int distance_x,distance_y;
  int distx,disty;

  distx=dista(player_x,point_x);
  disty=dista(player_y,point_y);
  if(distx<0)
    distx=-distx;
  distance_x=distx;

  if(disty<0)
    disty=-disty;
  distance_y=disty;

  
  if(distance_x<=SIGHTRANGE && distance_y<=SIGHTRANGE){
    *clflag=1;
    return (1);
  }else{
    return(0);
  }
  
}










/*移動方向取得の関数,非カノニカルモードとエコー防止を実装*/
char receptor(void)
{
  struct termios term,default_term;
  char c;

  tcgetattr(fileno(stdin),&term);
  
  default_term=term; //現在設定の退避
  
  term.c_lflag&=~ICANON; //非カノニカルに設定
  term.c_lflag&=~ECHO; //エコーを防止
  
  tcsetattr(fileno(stdin),TCSANOW,&term);
  
  c=getchar();

  tcsetattr(fileno(stdin),TCSANOW,&default_term);
  
  return c;
  
}



void debug(void)
{

  int i;

  printf("in map[%d][%d]\n",player_y,player_x);
  printf("blocks=%d\n",pathtest.blockvol);
  for(i=0;i<pathtest.blockvol;i++){
    printf("center of [%d]=map[%d][%d]\t",i,pathtest.blockcentery[i],pathtest.blockcenterx[i]);
    printf("pathway of [%d]=%d\t",i,pathtest.pathway[i]);
    printf("pathpoint of [%d]=%d\n",i,pathtest.pathp[i]);
    printf("%d and %d are connected.\t",i,pathtest.connect[i]);
    printf("receptway of [%d]=%d\t",i,pathtest.receptway[i]);
    printf("receptpoint of [%d]=%d\n",i,pathtest.receptp[i]);

  }

}
