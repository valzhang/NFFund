#include "KData.h"
using namespace std;

//����ʱ�䷶Χ�����������ʱ���Ƿ��ǺϷ�ʱ�䣬���浽����Ĳ�����
void ReadTime( int &begin, int &end );

//��TickerMap�ļ��ж������е��ڻ����룬���浽futureCode�����ڣ��������ڻ���������
int GetFutureCode( string path, string *code );

//��TickerMap�ļ��ж������е��ڻ������Ӧ�Ľ������Ĵ��룬���浽exchange�����ڣ��������ڻ���������
int GetFutureExchange(string path, string* exchange);

//��ĳ���ڻ�����\info\Xcthisc�ļ��ж�ȡĿ��ʱ���ڵ��ڻ���Լ��������Tick�ļ�����
void FutureProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &dataLog, ofstream &tickLog );

//��ȡ��Ӧ��Tick���ݲ�������ļ�
int ReadContractTick( KData *pData, int time, string exchange, string code, string contract, ofstream &dataLog );

//����ĳ���ڻ�ĳ��K��������Լ�ļ�
void MainContractProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &out, int gap );

//��ȡ��begin��end���ʱ���ں�ԼΪcontract��K�����ݣ��������out��
void ReadAndSaveMainData( KData *pData,string exchange, string code, int begin, int end, ofstream &out, int gap, string contract );

//��ȡ��begin��endʱ�䷶Χ��contract��Լ��K������Ȼ�����ϵ�һ���ļ���
void ContractProcess( KData *pData, string exchange, string code, string contract, int begin, int end, int gap );

//ɾ��ָ���ļ���·�����ļ�date����֮������
void ClearData( string path, int date );

string GetReadPath( string path, string exchange, string contract, char* chTime );

//����date����code�ڻ���������Լ
string GetMainContract( map<int, string>& mainMap, int date );

//������һ��
int GetNextDate( int date );
string Global_dataPath;
string Global_confPath;
string Global_outPath;
string Global_log="log\\";
string Global_pro_data = "pro_data\\";
string Global_contract = "contract\\";
string Global_main = "main\\";
//�ڻ�Ʒ����Ŀ
const int CONTRACT_NUM = 50;

const int TIMESTART = 20100101;

int main()
{
	
	ifstream confIn( "path.conf" );
	string dataPath;
	getline( confIn, dataPath );
	string confPath;
	getline( confIn, confPath );
	string outPath;
	getline( confIn, outPath);

	Global_dataPath = dataPath;
	Global_confPath = confPath;
	Global_outPath = outPath;

	string tmpPath = Global_outPath;
	if(CreateDirectory(tmpPath.c_str(),NULL))
	{
		cout<<"Create Output Directory OK!"<<endl;
	}
	
	tmpPath = Global_outPath;
	if(CreateDirectory(tmpPath.append(Global_log).c_str(), NULL))
	{
		cout<<"Create Output Log Directory OK!"<<endl;
	}

	tmpPath = Global_outPath;
	if(CreateDirectory(tmpPath.append(Global_pro_data).c_str(), NULL))
	{
		cout<<"Create Output pro_data Directory OK!"<< endl;
	}

	tmpPath = Global_outPath;
	if(CreateDirectory(tmpPath.append(Global_contract).c_str(), NULL))
	{
		cout<<"Create Output contract Directory OK!"<<endl;
	}

	tmpPath = Global_outPath;
	if(CreateDirectory(tmpPath.append(Global_main).c_str(), NULL))
	{
		cout<<"Create Output main Directory OK!"<<endl;
	}

	KData *pData = new KData();
	
	pData->SetTransferPeriod( 3 );
	pData->SetTransferTime( 1, 900, 1015 );
	pData->SetTransferTime( 2, 1030, 1130 );
	pData->SetTransferTime( 3, 1330, 1500 );
//	pData->SetTransferTime( 4, 1600, 2200 );

	pData->SetKLineNum( 6 );
	pData->SetKLine( 1, -1 );
	pData->SetKLine( 2, 1 );
	pData->SetKLine( 3, 2 );
	pData->SetKLine( 4, 5 );
	pData->SetKLine( 5, 15 );
	pData->SetKLine( 6, 30 );

/*
	pData->SetKLineNum( 1 );
	pData->SetKLine( 1, -1 );
*/
	if ( !pData->GetReady() )//GetReady()���pData��K�߲���������ʱ����Ƿ�����������
	{
		cout << "��������\n";
		return -1;
	}
	
//	ReadTime( beginTime, endTime );
	//ȡ�ý�������today
	time_t curtime=time(0); 
	tm tim =*localtime(&curtime); 
	int day,mon,year; 
	day=tim.tm_mday;
	mon=tim.tm_mon;
	year=tim.tm_year;
	int today = ( year + 1900 ) * 10000 + ( mon + 1) * 100 + day;
	
	//��TickerMap�ļ��ж������е��ڻ�����,���浽futureCode������
	string *futureCode = new string[CONTRACT_NUM];
	string *futureExchange = new string[CONTRACT_NUM];

	int futureNum;
	futureNum = GetFutureCode( confPath, futureCode );

	if(GetFutureExchange(confPath, futureExchange) != futureNum)
	{
		cout<< "�ڻ����״���ͽ��������ܶ�Ӧ\n"<<endl;
		return -1;
	}

	map<int, string> mainMap[CONTRACT_NUM];
	//����������Լ��Ϣ
	for ( int i = 0; i < futureNum; i++ ){

		//��roll�ļ�
		string mainPath = Global_confPath;
		mainPath.append("rolls\\");
		mainPath.append(futureCode[i]);
		mainPath.append(".rolls");
		ifstream inRolls;
		inRolls.open(mainPath);
		if (!inRolls){
			cout << "rolls�ļ���ʧ�ܣ�\n";
			continue;
		}

		string tmpData;
		getline( inRolls, tmpData );
		int last_date = 0;
		string last_contract;
		while (getline( inRolls, tmpData )){
			int date = atoi(tmpData.substr( 0, 8 ).c_str());
			string contract = tmpData.substr( 9, futureCode[i].length() + 4 );
			if ( last_date != 0 ){
				mainMap[i].insert(make_pair( date, last_contract ));
			}//if
			last_date = date;
			last_contract = contract;
		}//while
		mainMap[i].insert(make_pair( GetNextDate(today), last_contract ));

	}//for

	//����ʱ�䷶Χ
	int beginTime = 0;
	int endTime = 0;
	

	//��ȡfininshdate.txt�ļ�
	string datePath = Global_outPath;
	datePath.append( "finishdate.txt" );
	ifstream dateIn;
	dateIn.open( datePath );
	int finishDate = TIMESTART;
	
	if ( dateIn ){
		string finishDateStr;
		while ( getline( dateIn, finishDateStr ) ){}
		finishDate = atoi( finishDateStr.c_str() );
		if ( !finishDateStr.empty() ){
			finishDate = atoi( finishDateStr.c_str() );
			finishDate = GetNextDate( finishDate );
		}
		//ɾ����finishDate��today������
		cout << "�Ƿ�ɾ��" << finishDate << "����֮�������?(Y/N)" << endl;
		string delStr;
		cin >> delStr;
		if ( delStr.compare("Y") == 0 || ( delStr.compare("y") == 0 ) ){
			string tmpStr = Global_outPath;
			tmpStr.append( Global_main );
			ClearData( tmpStr, finishDate );
			tmpStr = Global_outPath;
			tmpStr.append( Global_contract );
			ClearData( tmpStr, finishDate );

		}

	}else
	dateIn.close();

	
	ofstream tickLog;
	string tickLogPath = Global_outPath;
	tickLogPath.append(Global_log);
	tickLogPath.append( "tick_miss_log" );

	tickLog.open( tickLogPath, ios::app);
	
	ofstream dataLog;
	string dataLogPath = Global_outPath;
	dataLogPath.append(Global_log);
	dataLogPath.append( "tick_error_log" );

	dataLog.open(dataLogPath, ios::app);
	


	//�����finishDate��today����������
		while ( finishDate < today){
			ofstream timeOut;
			timeOut.open("E:\\KData\\time.txt", ios::app);
			dataLog << "***************" << finishDate << "*****************\n";
			tickLog << "***************" << finishDate << "*****************\n";
			cout << "***************" << finishDate << "*****************\n";
			timeOut << "***************" << finishDate << "*****************\n";
			clock_t clock_begin = clock();
			beginTime = finishDate;
			endTime = finishDate;
			//��������
				for ( int i = 0; i < futureNum; i++ ){
					//debug
	//				if (futureCode[i].compare("ZN") != 0){
	//					continue;
	//				}
					if ( futureCode[i].compare("IF") == 0 ){
						pData->SetTransferPeriod( 2 );
						pData->SetTransferTime( 1, 915, 1130 );
						pData->SetTransferTime( 2, 1300, 1515 );
					}else{
						pData->SetTransferPeriod( 3 );
						pData->SetTransferTime( 1, 900, 1015 );
						pData->SetTransferTime( 2, 1030, 1130 );
						pData->SetTransferTime( 3, 1330, 1500 );
					}
					//����ÿһ���ڻ�����\info\Xcthisc�ļ��ж�ȡĿ��ʱ���ڵ��ڻ���Լ
					FutureProcess( pData,futureExchange[i], futureCode[i], beginTime, endTime, dataLog, tickLog );
				}

		for ( int i = 0; i < futureNum; i++ ){
			for ( int j = 0; j < pData->GetLineNum(); j++ ){
				string mainPath = outPath;
				mainPath.append(Global_main);
				mainPath.append( futureCode[i]);
				mainPath.append("_");
				int gap = pData->GetKLine( j+1 );
				if ( gap < 0 ){
					mainPath.append( "day" );
				}else{
					char chK[3];
					itoa( gap, chK, 10 );
					mainPath.append( chK );
				}
				mainPath.append( ".txt" );
				ofstream mainOut;
				mainOut.open( mainPath, ios::app);
				MainContractProcess( pData,futureExchange[i], futureCode[i], beginTime, endTime, mainOut, gap );
				mainOut.close();
			}
	
		}
	
		//���ʱ��������н��׺�Լ��K��
		for ( int i = 0; i < futureNum; i++ ){
			string contractPath = confPath;
			contractPath.append( "info\\" );
			contractPath.append( futureCode[i] );
			contractPath.append( "contracts" );
			ifstream inContract;
			inContract.open( contractPath );
			if ( !inContract ){
				cout << "code = " << futureCode[i] << ", contract�ļ���ȡʧ��\n";
				continue;
			}
			string strData;
			getline( inContract, strData );
		
			while ( getline( inContract, strData ) ){
				string contract = strData.substr( 0, futureCode[i].length()+4 );
				int b_contract = atoi( strData.substr( strData.length() - 17, 8 ).c_str() );
				int e_contract = atoi( strData.substr( strData.length() - 8, 8 ).c_str() );
				if ( b_contract > endTime ){
					break;
				}
				if ( e_contract < beginTime ){
					continue;
				}
				int begin = b_contract > beginTime ? b_contract : beginTime;
				int end = e_contract < endTime ? e_contract : endTime;		//��begin��end��������Ŀ��contract����������ʱ�䷶Χ�ڵĽ���ʱ�䷶Χ
			
				for ( int j = 0; j < pData->GetLineNum(); j++ ){
					ContractProcess( pData, futureExchange[i], futureCode[i], contract, begin, end, pData->GetKLine( j+1 ) );
				}

			}//while
		}
		clock_t clock_end = clock();
		timeOut << (float)(clock_end - clock_begin)/1000.0 << '\n';
		timeOut.close();
		ofstream dateOut;
		dateOut.open(datePath);
		dateOut << endl << finishDate ;
		dateOut.close();
		finishDate = GetNextDate( finishDate );
		
			dataLog << "===============" << finishDate << "===============\n";
			tickLog << "===============" << finishDate << "===============\n";
	}
	string todayContract[CONTRACT_NUM];

	//��ȡ���콻�׵����к�Լ�������浽����todayContract�У���¼���콻�׵����н��׺�Լ���������todayNum��
/*	for ( int i = 0; i < futureNum; i++ ){
		string futurePath = Global_confPath;
		futurePath.append( "info\\" );
		futurePath.append( futureCode[i] );
		futurePath.append( "cthist" );
		ifstream inFuture( futurePath );
		string futureData;
		int contractLen = futureCode[i].length() + 4;
		while ( getline( inFuture, futureData ) );
		int endDate = atoi( futureData.substr( 0, 8 ).c_str() );
		futureData = futureData.substr( 9, futureData.length() - 9 );
		if ( endDate == today ){
			while ( futureData.length() > 4 ){
				todayContract[todayNum] = futureData.substr( 0, contractLen );
				todayNum++;
				futureData = futureData.substr( contractLen + 1, futureData.length() - contractLen - 1 );
			}
		}

	}//for ( int i = 0; i < futureNum; i++ )
*/
	for ( int i = 0; i < futureNum; i++ ){
		string futurePath = Global_confPath;
		futurePath.append( "rolls\\" );
		futurePath.append( futureCode[i] );
		futurePath.append( ".rolls" );
		ifstream inFuture( futurePath );
		string futureData, tmp, lastContract;
		while ( getline( inFuture, tmp ) ){
			if ( tmp.length() > 0 ){
				futureData = tmp;
				int b_date = atoi( futureData.substr( 0, 8 ).c_str() );
				if ( b_date > today ){
					break;
				}
				lastContract = futureData.substr( 9, futureCode[i].length() + 4 );
			}
		}
		todayContract[i] = lastContract;
	}//for ( int i = 0; i < futureNum; i++ )

	clock_t lastTime;
	int tickTag[CONTRACT_NUM];
	memset( tickTag, 0, sizeof(int)*CONTRACT_NUM );
	KData *tickData[CONTRACT_NUM];
	for ( int i = 0; i < futureNum; i++ ){
		if ( futureCode[i].compare("IF") == 0 ){
			tickData[i] = new KData();
			tickData[i]->SetTransferPeriod( 2 );
			tickData[i]->SetTransferTime( 1, 915, 1130 );
			tickData[i]->SetTransferTime( 2, 1300, 1515 );
		}else{
			tickData[i] = new KData();
			tickData[i]->SetTransferPeriod( 3 );
			tickData[i]->SetTransferTime( 1, 900, 1015 );
			tickData[i]->SetTransferTime( 2, 1030, 1130 );
			tickData[i]->SetTransferTime( 3, 1330, 1500 );
			//tickData[i]->SetTransferTime( 4, 1600, 2200 );
		}
		tickData[i]->SetKLineNum( 6 );
		tickData[i]->SetKLine( 1, -1 );
		tickData[i]->SetKLine( 2, 1 );
		tickData[i]->SetKLine( 3, 2 );
		tickData[i]->SetKLine( 4, 5 );
		tickData[i]->SetKLine( 5, 15 );
		tickData[i]->SetKLine( 6, 30 );
		tickData[i]->GetReady();
	}
	
	char chToday[9];
	itoa( today, chToday, 10 );

	//���㱣������������Լ
	string todayMain[CONTRACT_NUM];
	for ( int i = 0; i < futureNum; i++ ){
		todayMain[i] = GetMainContract( mainMap[i], today );
	}//for
	//��¼�ϴ�������Լ���λ��
	int lastMainTime[CONTRACT_NUM][7];
	memset( lastMainTime, 0, sizeof(lastMainTime) );

	while ( finishDate == today ){
		//���µ�ǰʱ��curTime
		lastTime = clock();
		cout << "Today Loop Time\n";
		//����ÿһ�ֺ�Լ����ȡ��Ӧ��Tick�ļ�������K���ļ���
		for ( int i = 0; i < futureNum; i++ ){
			//��ȡTick�ļ�·��
			string tickPath = GetReadPath( Global_dataPath, futureExchange[i], todayContract[i], chToday );
			ifstream inToday;
			inToday.open(tickPath);
			if ( !inToday ){
				transform( todayContract[i].begin(), todayContract[i].end(), todayContract[i].begin(), tolower );	//���Զ�ȡСд�ļ���
				tickPath = GetReadPath(Global_dataPath, futureExchange[i], todayContract[i], chToday );
				inToday.open( tickPath );
				if ( !inToday ){
					continue;
				}
			}
			int tag = 0;
			string tmp;
			while ( tag < tickTag[i] ){
				getline( inToday, tmp );
				tag++;
			}
			double oneData[28];
			while ( !inToday.eof() ){
				for ( int j = 0; j < 27; j++ ){
					inToday >> oneData[j];
				}
				tickData[i]->AddData( todayContract[i], today, oneData );
				//cout << "Add Data";
		//		cout << "��ȡһ��Tick���ݳɹ���\n";
				tickTag[i]++;
			}//while ( !inToday.eof() ){
			//
			tmp = Global_outPath;
			tickData[i]	->PrintData( tmp.append( Global_pro_data ) , todayContract[i], chToday );
			//cout << "\nPrint Data\n";
			//�жϵ�ǰcontract�Ƿ�Ϊ������Լ
			cout << todayContract[i] << ' ' << todayMain[i] <<endl;
			transform( todayContract[i].begin(), todayContract[i].end(), todayContract[i].begin(), toupper );	
			transform( todayMain[i].begin(), todayMain[i].end(), todayMain[i].begin(), toupper );	
			if ( todayContract[i].compare(todayMain[i]) == 0 ){
			//����������Լ���򲹳�K�����ݵ�main�ļ�ĩβ
			//todo 
				cout << "MainContract\n";
				for ( int j = 1; j < tickData[i]->GetLineNum(); j++ ){	//����ÿ��K��
					if ( lastMainTime[i][j] + tickData[i]->GetKLine(j) <= (int)(oneData[0]/100) ){		//���ϴ��������ǰTick����ʱ���main�ļ���ʱ��+K�߼��ʱ��Ҫ��
						//������һ��K�߼�¼�����main�ļ�
						cout << "OutMain\n";
						string mainContractPath = Global_outPath;
						mainContractPath.append("main\\");
						mainContractPath.append(futureCode[i]);
						mainContractPath.append("_");
						if ( tickData[i]->GetKLine(j) < 0 ){
							mainContractPath.append("day.txt");
						}else{
							char chK[3];
							itoa( tickData[i]->GetKLine(j), chK, 10 );
							mainContractPath.append(chK);
							mainContractPath.append(".txt");
						}//if
						ofstream mainContractOut;
						mainContractOut.open( mainContractPath, ios::app );
					
						if ( tickData[i]->k_index[j] > 0 ){
							tickData[i]->PrintLastData( mainContractOut, todayContract[i], j );
							cout << "����main�ļ��ɹ���\n";
						}//if
						mainContractOut.close();
						//�������¼�¼K��
						lastMainTime[i][j] = (int)( oneData[0] / 100 );
					}//if
				}//for
			}//if

		}//for ( int i = 0; i < todayNum; i++ )
		while ( clock() - lastTime < 5000 ){
			Sleep(5000);
		}
		//sleepֱ��curTime+20s
	}
	

		pData->DeleteData();

	dataLog.close();
	tickLog.close();




	delete pData;
	
	return 0;

}
//�����������Ƿ���һ���Ϸ���ʱ��
bool IsAFutureDate( int t )
{
	if ( t < 19901012 || t > 20991231){
		return false;
	}
	int month = ( t % 10000 ) / 100;
	if ( month == 0 || month > 12 ){
		return false;
	}
	int day = t % 100;
	if ( day == 0 || day > 31 ){
		return false;
	}
	return true;
}

//������������ʱ���Ƿ�Ϸ�
bool TestTime( int b, int e )
{
	if ( b <= e && IsAFutureDate(b) && IsAFutureDate(e) ){
		return true;
	}else{
		return false;
	}
}

void ReadTime ( int &begin, int &end )
{
	do{
		cout << "���������������ʱ����ʼֵ(yyyymmdd)" << endl;
		cin >> begin;
		cout << "���������������ʱ�����ֵ(yyyymmdd)" << endl;
		cin >> end;

		if ( TestTime(begin, end) ){
			break;
		}else{
			cout << "����ʱ�����ݸ�ʽ��������������" << endl;
		}
	}while (1);

}

int GetFutureCode( string path, string *code )
{
	ifstream in;
	string codePath = path;
	codePath.append( "TickerMap" );
	in.open( codePath );
	string tmp;
	getline( in, tmp );
	int index = 0;
	while ( getline (in, tmp) ){
		code[index] = tmp.substr( 0, tmp.find( '|', 0 ) );
 		index++;
	}
	in.close();
	return index;
}

int GetFutureExchange(string path, string* exchange)
{
	ifstream in;
	string codePath = path;
	codePath.append( "TickerMap" );
	in.open( codePath );
	string tmp;
	getline( in, tmp );
	int index = 0;
	while ( getline (in, tmp) ){
		int count = 0;
		while(tmp.find('|', 0)!=-1 &&  count<=5)
		{
			count = count + 1;
			tmp = tmp.substr(tmp.find( '|', 0 )+1, tmp.length()-tmp.find('|', 0));
		}
		exchange[index] = tmp.substr( 0, tmp.find( '|', 0 ) );
		index++;
	}
	in.close();
	return index;
}

//��ԭʼTick�ļ�����pro_data�����K���ļ���
void FutureProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &dataLog, ofstream &tickLog )
{
	clock_t start_time;
	clock_t stop_time;
	

	//����ÿһ���ڻ�����\info\Xcthisc�ļ��ж�ȡĿ��ʱ���ڵ��ڻ���Լ
	string futurePath = Global_confPath;
	futurePath.append( "info\\" );
	futurePath.append( code );
	futurePath.append( "cthist" );

	ifstream inFuture;
	inFuture.open( futurePath );
	string strData;
	if ( inFuture ){
		while ( getline( inFuture, strData ) ){
			int time = atoi( strData.substr( 0, 8 ).c_str() );
			strData = strData.substr( 9, strData.length() - 9 );
			if ( time >= begin && time <= end){		//������м�¼��ʱ�䷶Χ�ڣ�����
				while ( strData.length() > 4 ){		//������к�Լ��û�д���
					int pos = strData.find( '|', 0 );
					if ( pos != string::npos ){		//����¼��ֹһ����Լ
						string contract = strData.substr( 0, pos );
						strData = strData.substr( code.length()+5, strData.length()-code.length()-5 );
						//��ȡ����ʱ��ú�Լ��Tick���ݲ�����������ļ�
						start_time = clock();
						if ( ReadContractTick( pData, time, exchange, code, contract, dataLog ) < 0 ){
							//����ȡ���ɹ����������log�ļ�
							tickLog << " time = " << time 
									<< " code = " << code
									<< " contract = " << contract 
									<< endl;
						}
						stop_time = clock();
						cout <<exchange<<" "<<code<< " Cal Time = " << (stop_time - start_time) << "ms" << endl;
					}else{							//����¼��ֻʣһ����Լ
						start_time = clock();
						if ( ReadContractTick( pData,  time, exchange, code, strData, dataLog ) < 0 ){
							//����ȡ���ɹ����������log�ļ�
							tickLog << " time = " << time 
									<< " code = " << code
									<< " contract = " << strData 
									<< endl;
						}
						stop_time = clock();
						cout <<exchange<<" "<<code<< " Cal Time = " << (stop_time - start_time) << "ms" << endl;
						break;
					}
				}
			}
			if ( time > end ){		//������м�¼��������Ҫ��ʱ�䷶Χ֮�󣬽�����ȡѭ��
				break;
			}
		}
	}else
	{
		cout << "Read Future = " << code << " Error!\n";
	}
}

string GetReadPath( string path, string exchange, string contract, char* chTime )
{
	string readPath = path;
	if(exchange=="DCE")
	{
		readPath.append("DCE\\");
	}else if(exchange=="SHFE")
	{
		readPath.append("SHF\\");
	}
	else if(exchange == "CZCE")
	{
		readPath.append("CZC\\");
	}else if (exchange == "CFFE")
	{
		readPath.append("CFFE\\");
	}
	
	char year[5];
	memset(year, 0, 5);
	memcpy(year, chTime, 4);

	readPath.append(year);
	readPath.append("\\");
	if (exchange.compare("CZCE") == 0){
		readPath.append( contract.substr(0, contract.length() - 4) );
		readPath.append( contract.substr(contract.length() - 3, 3) );
	}else{
		readPath.append( contract );
	}
	readPath.append( "_" );
	readPath.append( chTime );
	readPath.append( "_5.txt" );

	return readPath;
}

int ReadContractTick( KData *pData, int time, string exchange, string code, string contract, ofstream &dataLog )
{
	//��ȡTick�ļ�����ʧ�ܣ�����-1
	char chTime[9];
	itoa( time, chTime, 10 );
	string readPath = GetReadPath(Global_dataPath, exchange, contract, chTime );
	ifstream in;
	in.open( readPath );
	if ( !in ){
		transform( contract.begin(), contract.end(), contract.begin(), tolower );	//���Զ�ȡСд�ļ���
		readPath = GetReadPath(Global_dataPath, exchange, contract, chTime );
		in.open( readPath );
		if ( !in ){		//�����Ȼ��ȡ����������˵���Ҳ���Ŀ��Tick�ļ�������-1
	//		cout<<readPath<<endl;
			return -1;
		}

	}


	//����ÿһ�����ݶ�ȡ�����浽KData���ݽṹ�У������ݴ����������log�ļ�������
	bool flag = true;	//��Ǳ��������Ƿ���Ч
	double tickData[28];
	while ( !in.eof() ){	
		//��ȡһ������,���и�ֵ�����ǣ�����ѭ��������Tick���ݶ��뵽����K��������
		for ( int i = 0; i < 27; i++ ){
			in >> tickData[i];
			if ( tickData[i] < 0 ){
				flag = false;
				dataLog << "time = " << time 
						<< "code = " << code
						<< "contract = " << contract 
						<< endl;
			}
		}
		
		if ( !flag ){
			break;
		}else{
			pData->AddData( contract, time, tickData );
		}

	}



	if ( flag ){
		pData->FillData();
		string tmpPath = Global_outPath;
		pData->PrintData(tmpPath.append(Global_pro_data), contract, chTime );
	}

	cout << contract << '\t' << time << "\t����ɹ���\n";
	//����
	pData->ClearData();
	return 0;
}

void MainContractProcess( KData *pData,string exchange, string code, int begin, int end, ofstream &out, int gap )
{
	ifstream inRoll;
	string rollPath = Global_confPath;
	rollPath.append( "rolls\\" );
	rollPath.append( code );
	rollPath.append( ".rolls" );
	inRoll.open( rollPath );
	if ( !inRoll ){
		cout << "code = " << code << "������Լ�����ļ���ȡʧ��\n";
		return;
	}
	string strData;
	getline( inRoll, strData );
	string lastContract;
	int time;
	while( getline( inRoll, strData ) ){
		time = atoi( strData.substr( 0, 8 ).c_str() );
		
		string contract = strData.substr( 9, code.length() + 4 );
		if ( time > end ){
			//�����begin��end��������Լ��������Լ=lastContract
			ReadAndSaveMainData( pData, exchange, code, begin, end, out, gap, lastContract );
			break;
		}else{
			if ( time > begin ){
				//�����begin��time(����)��������Լ=lastContract
				ReadAndSaveMainData( pData, exchange, code, begin, time, out, gap, lastContract );
				begin = time;
			}
		}
		lastContract = contract;
	}//while
	if ( time <= begin ){
		ReadAndSaveMainData( pData, exchange, code, begin, end, out, gap, lastContract );
	}
}

void ReadAndSaveMainData( KData *pData, string exchange, string code, int begin, int end, ofstream &out, int gap, string contract )
{
	string mainMissPath = Global_dataPath;
	mainMissPath.append( "main_miss_log" );
	ofstream mainLog( mainMissPath ,ios::app);

	ifstream inInfo;
	string infoPath = Global_confPath;
	infoPath.append( "info\\" );
	infoPath.append( code );
	infoPath.append( "cthist" );
	inInfo.open( infoPath );
	if ( !inInfo ){
		cout << "code = " << code << "info�ļ���ȡ����\n";
		return;
	}

	ifstream in;
	string strData;
	while ( getline( inInfo, strData ) ){
		int time = atoi( strData.substr( 0, 8 ).c_str() );		
		if ( time < begin ){
			continue;
		}
		if ( time > end ){
			break;
		}
		string readPath = Global_outPath;
		readPath.append( "pro_data\\" );
		readPath.append( contract );
		readPath.append( "_" );
		char chTime[9];
		itoa( time, chTime, 10 );
		readPath.append( chTime );
		readPath.append( "_" );
		if ( gap < 0 ){
			readPath.append( "day" );
		}else{
			char chK[3];
			itoa( gap, chK, 10 );
			readPath.append( chK );
		}
		readPath.append( ".txt" );
		in.open( readPath, ios::binary );
		if ( !in ){
			mainLog << "time = " << time << ", contract = " << contract << ", k = " << gap << ", cannot find Contract!\n";
			continue;
		}else{
			out << in.rdbuf();
			cout << "main " << contract << "\t" << time << "\t����ɹ���\n";
		}
		in.clear();
		in.close();
	}//while
}

void ContractProcess( KData *pData, string exchange, string code, string contract, int begin, int end, int gap )
{
	ofstream contractLog;
	string contractMissPath = Global_confPath;
	contractMissPath.append( "contract_miss_log" );
	contractLog.open( contractMissPath );

	int flag = 0;
	ofstream out;
	string contractPath = Global_outPath;
	contractPath.append( "contract\\");
	contractPath.append( contract );
	contractPath.append( "_" );
	if ( gap < 0 ){
		contractPath.append ( "day" );
	}else{
		char chK[3];
		itoa( gap, chK, 10 );
		contractPath.append( chK );
	}
	contractPath.append( ".txt" );
	out.open( contractPath, ios::app );

	ifstream inInfo;
	string infoPath = Global_confPath;
	infoPath.append( "info\\" );
	infoPath.append( code );
	infoPath.append( "cthist" );
	inInfo.open( infoPath );
	if ( !inInfo ){
		cout << "code = " << code << ", info�����ļ���ȡʧ��\n";
		return;
	}

	string strData;
	while ( getline( inInfo, strData ) ){
		int time = atoi( strData.substr( 0, 8 ).c_str() );
		if ( time < begin ){
			continue;
		}
		if ( time > end ){
			break;
		}
		char chTime[9];
		itoa( time, chTime, 10 );
		ifstream in;
		string readPath = Global_outPath;
		readPath.append( "pro_data\\" );
		readPath.append( contract );
		readPath.append( "_" );
		readPath.append( chTime );
		readPath.append( "_" );
		if ( gap < 0 ){
			readPath.append( "day" );
		}else{
			char chK[3];
			itoa( gap, chK, 10 );
			readPath.append( chK );
		}
		readPath.append( ".txt" );
		in.open( readPath, ios::binary );
		if ( !in ){
			contractLog << "contract = " << contract
				<< ", time = " << time
				<< ", k = " << gap
				<< ", cannot find data\n";		
			in.clear();
			in.close();
			continue;
		}else{
			flag = 1;
			out << in.rdbuf();
			cout << "contract " << contract << "\t" << time << "\t����ɹ���\n";
		}

		in.clear();
		in.close();
	}//while
	out.clear();
	out.close();
	if ( flag == 0 ){
		DeleteFile( contractPath.c_str() );
	}
}

//������һ��
int GetNextDate( int date )
{
	int year = date / 10000;
	int month = ( date / 100 ) % 100;
	int day = date % 100;

	switch (day){
	case 28:
		if ( month != 2 ){
			day++;
		}else if ( year % 4 == 0 ){
			day++;
		}else{
			day = 1;
			month++;
		}
		break;
	case 29:
		if ( month == 2 ){
			day = 1;
			month++;
		}else{
			day++;
		}
		break;
	case 30:
		if ( month == 4 || month == 6 || month == 9 || month == 11 ){
			day = 1;
			month++;
		}else{
			day++;
		}
		break;
	case 31:
		day = 1;
		month++;
		break;
	default:
		day++;
		break;
	}//switch

	if ( month > 12 ){
		month = 1;
		year++;
	}

	date = day + month * 100 + year * 10000;
	return date;
}

//ɾ��ָ���ļ���·�����ļ�date����֮������
void ClearData( string path, int endDate )
{
	char szFind[1024];
	char szFile[1024];
	
	WIN32_FIND_DATA FindFileData;

	strcpy( szFind, path.c_str() );
	strcat( szFind, "*.*");

	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if ( INVALID_HANDLE_VALUE == hFind ){
		return;
	}
	while ( true ){
		if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
		}else{
			string filePath = path;
			filePath.append( FindFileData.cFileName );
			ifstream fileIn;
			fileIn.open( filePath );
			ofstream fileOut;
			string fileTmpStr = Global_outPath;
			fileTmpStr.append( "tmp.txt" );
			fileOut.open( fileTmpStr );
			string dataStr;
			while ( getline( fileIn, dataStr ) ){
				int date = atoi( dataStr.substr( filePath.length() + 5, 9 ).c_str() );

				if ( date >= endDate ){
					break;
				}
				fileOut << dataStr << endl;				
			}
			fileIn.close();
			fileOut.close();
			fileIn.open( fileTmpStr, ios::binary );
			fileOut.open( filePath );
			fileOut << fileIn.rdbuf();
			fileIn.close();
			fileOut.close();

		}
		if ( !FindNextFile( hFind, &FindFileData ) ){
			break;
		}
	}
	FindClose( hFind );
}

string GetMainContract( map<int, string>& mainMap, int date )
{
	return mainMap.upper_bound(date)->second;
}