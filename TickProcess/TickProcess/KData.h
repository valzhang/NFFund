#pragma once

#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <sstream>   
#include <algorithm>  


class KData
{
	int period_num;		//交易时间段个数
	int *transfer_time_begin;		//交易时间段起始
	int *transfer_time_end;			//交易时间段结束
	int k_line_num;			//K线种类个数
	int *k_gap;				//K线分钟数
	int transfer_minute;		//每天交易总分钟数
	int data_num;				//分钟K线数据类型个数

	double k_1_mean;		//过去10个1分钟K线收盘价均值
	double k_5_mean;		//过去6个5分钟K线收盘价均值
/*
0	int **date;		//日期
1	int **time;		//分钟
2	double **open;		//开盘价
3	double **close;		//收盘价
4	double **max;		//最高价
5	double **min;		//最低价
6	double **buy;		//买一价
7	int **buy_vol;		//买一量
8	double **sell;		//卖一价
9	int **sell_vol;		//卖一量
10	long long **transfer_vol;	//交易量
11	double **transfer_amount;	//交易金额
12	double **total_vol;		//累计交易量
13	double **total_amount;	//累计交易金额
14	long long **position;	//持仓量
	*/
	double ***k_data;		//kdata[p][q][r]p代表数据类型，取值范围0~(data_num-1)；q代表k线种类，取值-1为日线，r代表当天第r个k线数据


public:
	KData(void);
	~KData(void);

	//设置交易时间段个数
	void SetTransferPeriod( int n );
	//设置第n个交易时间段
	bool SetTransferTime( int n, int begin, int end );
	//设置K线个数
	void SetKLineNum( int n );
	//获取K线个数
	int GetLineNum();
	//设置第n个K线参数
	void SetKLine( int n, int gap );
	//获得第n个K线参数
	int GetKLine( int n );
	//设置参数完毕，准备输入数据
	bool GetReady();


	//添加一个Tick数据
	bool AddData( std::string contract, int date, double *data );
	//删除所有Tick数据
	void ClearData();
	//补全空值数据
	void FillData();
	//输出数据到文件
	void PrintData( std::string path, std::string contract, char* chTime );
	//计算时间编号
	int CalMinuteIndex( int minute, int gap );
	//判断某个时间是否在交易时间范围内
	bool IsTransferTime( int time );
	//计算两个时间之间的分钟数
	int CalGapMin( int begin, int end );
	//计算最近的时间开始标记分钟
	int NearMinute( int minute, int gap );
	//返回下一分钟时间值
	int NextMinute( int minute );
	//返回上一分钟时间值
	int LastMinute( int minute );
	void DeleteData();
	
};

/*
#pragma once

#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <sstream>   
#include <algorithm>  


class KData
{
	int date_1[362];
	int date_day;
	int date_2[181];
	int date_5[73];
	int date_15[25];
	int date_30[13];

	int time_1[362];
	int time_day;
	int time_2[181];
	int time_5[73];
	int time_15[25];
	int time_30[13];

	double open_1[362];	
	double open_day;
	double open_2[181];
	double open_5[73];
	double open_15[25];
	double open_30[13];

	double close_1[362];
	double close_day;
	double close_2[181];
	double close_5[73];
	double close_15[25];
	double close_30[13];

	double max_1[362];
	double max_day;
	double max_2[181];
	double max_5[73];
	double max_15[25];
	double max_30[13];

	double min_1[362];
	double min_day;
	double min_2[181];
	double min_5[73];
	double min_15[25];
	double min_30[13];

	long long volume_1[362];
	long long volume_day;
	long long volume_2[181];
	long long volume_5[73];
	long long volume_15[25];
	long long volume_30[13];

	long long turnover_1[362];
	long long turnover_day;
	long long turnover_2[181];
	long long turnover_5[73];
	long long turnover_15[25];
	long long turnover_30[13];

	long long totalVolume_1[362];
	long long totalVolume_day;
	long long totalVolume_2[181];
	long long totalVolume_5[73];
	long long totalVolume_15[25];
	long long totalVolume_30[13];

	double totalTurnover_1[362];
	double totalTurnover_day;
	double totalTurnover_2[181];
	double totalTurnover_5[73];
	double totalTurnover_15[25];
	double totalTurnover_30[13];

	long long position_1[362];
	long long position_day;
	long long position_2[181];
	long long position_5[73];
	long long position_15[25];
	long long position_30[13];


public:
	KData(void);
	~KData(void);

	void FillData();
	bool AddData( int date, double *data );
	void PrintData( std::string path, std::string contract, char* chTime );
	void CalData( int i );
};

*/