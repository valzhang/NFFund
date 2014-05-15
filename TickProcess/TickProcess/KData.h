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
	int period_num;		//����ʱ��θ���
	int *transfer_time_begin;		//����ʱ�����ʼ
	int *transfer_time_end;			//����ʱ��ν���
	int k_line_num;			//K���������
	int *k_gap;				//K�߷�����
	int transfer_minute;		//ÿ�콻���ܷ�����
	int data_num;				//����K���������͸���

	double k_1_mean;		//��ȥ10��1����K�����̼۾�ֵ
	double k_5_mean;		//��ȥ6��5����K�����̼۾�ֵ
/*
0	int **date;		//����
1	int **time;		//����
2	double **open;		//���̼�
3	double **close;		//���̼�
4	double **max;		//��߼�
5	double **min;		//��ͼ�
6	double **buy;		//��һ��
7	int **buy_vol;		//��һ��
8	double **sell;		//��һ��
9	int **sell_vol;		//��һ��
10	long long **transfer_vol;	//������
11	double **transfer_amount;	//���׽��
12	double **total_vol;		//�ۼƽ�����
13	double **total_amount;	//�ۼƽ��׽��
14	long long **position;	//�ֲ���
	*/
	double ***k_data;		//kdata[p][q][r]p�����������ͣ�ȡֵ��Χ0~(data_num-1)��q����k�����࣬ȡֵ-1Ϊ���ߣ�r�������r��k������


public:
	KData(void);
	~KData(void);

	//���ý���ʱ��θ���
	void SetTransferPeriod( int n );
	//���õ�n������ʱ���
	bool SetTransferTime( int n, int begin, int end );
	//����K�߸���
	void SetKLineNum( int n );
	//��ȡK�߸���
	int GetLineNum();
	//���õ�n��K�߲���
	void SetKLine( int n, int gap );
	//��õ�n��K�߲���
	int GetKLine( int n );
	//���ò�����ϣ�׼����������
	bool GetReady();


	//���һ��Tick����
	bool AddData( std::string contract, int date, double *data );
	//ɾ������Tick����
	void ClearData();
	//��ȫ��ֵ����
	void FillData();
	//������ݵ��ļ�
	void PrintData( std::string path, std::string contract, char* chTime );
	//����ʱ����
	int CalMinuteIndex( int minute, int gap );
	//�ж�ĳ��ʱ���Ƿ��ڽ���ʱ�䷶Χ��
	bool IsTransferTime( int time );
	//��������ʱ��֮��ķ�����
	int CalGapMin( int begin, int end );
	//���������ʱ�俪ʼ��Ƿ���
	int NearMinute( int minute, int gap );
	//������һ����ʱ��ֵ
	int NextMinute( int minute );
	//������һ����ʱ��ֵ
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