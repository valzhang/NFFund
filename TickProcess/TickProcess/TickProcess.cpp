#include "KData.h"
using namespace std;

//读入时间范围，并检查输入时间是否是合法时间，保存到传入的参数中
void ReadTime( int &begin, int &end );

//从TickerMap文件中读入所有的期货代码，保存到futureCode数组内，并返回期货代码数量
int GetFutureCode( string path, string *code );

//从TickerMap文件中读入所有的期货代码对应的交易所的代码，保存到exchange数组内，并返回期货代码数量
int GetFutureExchange(string path, string* exchange);

//对某种期货，从\info\Xcthisc文件中读取目标时间内的期货合约，并进行Tick文件处理
void FutureProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &dataLog, ofstream &tickLog );

//读取相应的Tick数据并输出到文件
int ReadContractTick( KData *pData, int time, string exchange, string code, string contract, ofstream &dataLog );

//生成某种期货某种K线主力合约文件
void MainContractProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &out, int gap );

//读取从begin到end这段时间内合约为contract的K线数据，并输出到out中
void ReadAndSaveMainData( KData *pData,string exchange, string code, int begin, int end, ofstream &out, int gap, string contract );

//读取从begin到end时间范围内contract合约的K线数据然后整合到一个文件内
void ContractProcess( KData *pData, string exchange, string code, string contract, int begin, int end, int gap );

//删除指定文件夹路径下文件date日期之后数据
void ClearData( string path, int date );

string GetReadPath( string path, string exchange, string contract, char* chTime );

//查找date日期code期货的主力合约
string GetMainContract( map<int, string>& mainMap, int date );

//返回下一天
int GetNextDate( int date );
string Global_dataPath;
string Global_confPath;
string Global_outPath;
string Global_log="log\\";
string Global_pro_data = "pro_data\\";
string Global_contract = "contract\\";
string Global_main = "main\\";
//期货品种数目
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
	if ( !pData->GetReady() )//GetReady()检查pData的K线参数及交易时间段是否设置完整。
	{
		cout << "参数错误\n";
		return -1;
	}
	
//	ReadTime( beginTime, endTime );
	//取得今天日期today
	time_t curtime=time(0); 
	tm tim =*localtime(&curtime); 
	int day,mon,year; 
	day=tim.tm_mday;
	mon=tim.tm_mon;
	year=tim.tm_year;
	int today = ( year + 1900 ) * 10000 + ( mon + 1) * 100 + day;
	
	//从TickerMap文件中读入所有的期货代码,保存到futureCode数组内
	string *futureCode = new string[CONTRACT_NUM];
	string *futureExchange = new string[CONTRACT_NUM];

	int futureNum;
	futureNum = GetFutureCode( confPath, futureCode );

	if(GetFutureExchange(confPath, futureExchange) != futureNum)
	{
		cout<< "期货交易代码和交易所不能对应\n"<<endl;
		return -1;
	}

	map<int, string> mainMap[CONTRACT_NUM];
	//读入主力合约信息
	for ( int i = 0; i < futureNum; i++ ){

		//打开roll文件
		string mainPath = Global_confPath;
		mainPath.append("rolls\\");
		mainPath.append(futureCode[i]);
		mainPath.append(".rolls");
		ifstream inRolls;
		inRolls.open(mainPath);
		if (!inRolls){
			cout << "rolls文件打开失败！\n";
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

	//读入时间范围
	int beginTime = 0;
	int endTime = 0;
	

	//读取fininshdate.txt文件
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
		//删除从finishDate到today的数据
		cout << "是否删除" << finishDate << "日期之后的数据?(Y/N)" << endl;
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
	


	//计算从finishDate到today的所有数据
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
			//处理数据
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
					//对于每一种期货，从\info\Xcthisc文件中读取目标时间内的期货合约
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
	
		//输出时间段内所有交易合约的K线
		for ( int i = 0; i < futureNum; i++ ){
			string contractPath = confPath;
			contractPath.append( "info\\" );
			contractPath.append( futureCode[i] );
			contractPath.append( "contracts" );
			ifstream inContract;
			inContract.open( contractPath );
			if ( !inContract ){
				cout << "code = " << futureCode[i] << ", contract文件读取失败\n";
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
				int end = e_contract < endTime ? e_contract : endTime;		//从begin到end就是我们目标contract在我们所需时间范围内的交易时间范围
			
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

	//读取今天交易的所有合约，并保存到数组todayContract中，记录今天交易的所有交易合约种类个数到todayNum中
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

	//计算保存今天的主力合约
	string todayMain[CONTRACT_NUM];
	for ( int i = 0; i < futureNum; i++ ){
		todayMain[i] = GetMainContract( mainMap[i], today );
	}//for
	//记录上次主力合约输出位置
	int lastMainTime[CONTRACT_NUM][7];
	memset( lastMainTime, 0, sizeof(lastMainTime) );

	while ( finishDate == today ){
		//记下当前时间curTime
		lastTime = clock();
		cout << "Today Loop Time\n";
		//对于每一种合约，读取相应的Tick文件，处理到K线文件中
		for ( int i = 0; i < futureNum; i++ ){
			//获取Tick文件路径
			string tickPath = GetReadPath( Global_dataPath, futureExchange[i], todayContract[i], chToday );
			ifstream inToday;
			inToday.open(tickPath);
			if ( !inToday ){
				transform( todayContract[i].begin(), todayContract[i].end(), todayContract[i].begin(), tolower );	//尝试读取小写文件名
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
		//		cout << "读取一条Tick数据成功！\n";
				tickTag[i]++;
			}//while ( !inToday.eof() ){
			//
			tmp = Global_outPath;
			tickData[i]	->PrintData( tmp.append( Global_pro_data ) , todayContract[i], chToday );
			//cout << "\nPrint Data\n";
			//判断当前contract是否为主力合约
			cout << todayContract[i] << ' ' << todayMain[i] <<endl;
			transform( todayContract[i].begin(), todayContract[i].end(), todayContract[i].begin(), toupper );	
			transform( todayMain[i].begin(), todayMain[i].end(), todayMain[i].begin(), toupper );	
			if ( todayContract[i].compare(todayMain[i]) == 0 ){
			//若是主力合约，则补充K线数据到main文件末尾
			//todo 
				cout << "MainContract\n";
				for ( int j = 1; j < tickData[i]->GetLineNum(); j++ ){	//对于每种K线
					if ( lastMainTime[i][j] + tickData[i]->GetKLine(j) <= (int)(oneData[0]/100) ){		//若上次输出到当前Tick数据时间比main文件的时间+K线间隔时间要打
						//将最新一条K线记录输出到main文件
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
							cout << "更新main文件成功！\n";
						}//if
						mainContractOut.close();
						//更新最新记录K线
						lastMainTime[i][j] = (int)( oneData[0] / 100 );
					}//if
				}//for
			}//if

		}//for ( int i = 0; i < todayNum; i++ )
		while ( clock() - lastTime < 5000 ){
			Sleep(5000);
		}
		//sleep直到curTime+20s
	}
	

		pData->DeleteData();

	dataLog.close();
	tickLog.close();




	delete pData;
	
	return 0;

}
//检测输入参数是否是一个合法的时间
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

//检测输入的两个时间是否合法
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
		cout << "请输入待处理数据时间起始值(yyyymmdd)" << endl;
		cin >> begin;
		cout << "请输入待处理数据时间结束值(yyyymmdd)" << endl;
		cin >> end;

		if ( TestTime(begin, end) ){
			break;
		}else{
			cout << "输入时间数据格式错误，请重新输入" << endl;
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

//从原始Tick文件生成pro_data下面的K线文件。
void FutureProcess( KData *pData, string exchange, string code, int begin, int end, ofstream &dataLog, ofstream &tickLog )
{
	clock_t start_time;
	clock_t stop_time;
	

	//对于每一种期货，从\info\Xcthisc文件中读取目标时间内的期货合约
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
			if ( time >= begin && time <= end){		//如果该行记录在时间范围内，则处理
				while ( strData.length() > 4 ){		//如果还有合约还没有处理
					int pos = strData.find( '|', 0 );
					if ( pos != string::npos ){		//若记录不止一条合约
						string contract = strData.substr( 0, pos );
						strData = strData.substr( code.length()+5, strData.length()-code.length()-5 );
						//读取当天时间该合约的Tick数据并处理输出到文件
						start_time = clock();
						if ( ReadContractTick( pData, time, exchange, code, contract, dataLog ) < 0 ){
							//若读取不成功，则输出到log文件
							tickLog << " time = " << time 
									<< " code = " << code
									<< " contract = " << contract 
									<< endl;
						}
						stop_time = clock();
						cout <<exchange<<" "<<code<< " Cal Time = " << (stop_time - start_time) << "ms" << endl;
					}else{							//若记录里只剩一条合约
						start_time = clock();
						if ( ReadContractTick( pData,  time, exchange, code, strData, dataLog ) < 0 ){
							//若读取不成功，则输出到log文件
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
			if ( time > end ){		//如果该行记录在我们需要的时间范围之后，结束读取循环
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
	//读取Tick文件，若失败，返回-1
	char chTime[9];
	itoa( time, chTime, 10 );
	string readPath = GetReadPath(Global_dataPath, exchange, contract, chTime );
	ifstream in;
	in.open( readPath );
	if ( !in ){
		transform( contract.begin(), contract.end(), contract.begin(), tolower );	//尝试读取小写文件名
		readPath = GetReadPath(Global_dataPath, exchange, contract, chTime );
		in.open( readPath );
		if ( !in ){		//如果仍然读取不出来，则说明找不到目标Tick文件，返回-1
	//		cout<<readPath<<endl;
			return -1;
		}

	}


	//对于每一行数据读取并保存到KData数据结构中，若数据错误，则输出到log文件，返回
	bool flag = true;	//标记本次数据是否有效
	double tickData[28];
	while ( !in.eof() ){	
		//读取一行数据,若有负值，则标记，结束循环，否则将Tick数据读入到分钟K线数据中
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

	cout << contract << '\t' << time << "\t处理成功！\n";
	//返回
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
		cout << "code = " << code << "主力合约配置文件读取失败\n";
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
			//处理从begin到end的主力合约，主力合约=lastContract
			ReadAndSaveMainData( pData, exchange, code, begin, end, out, gap, lastContract );
			break;
		}else{
			if ( time > begin ){
				//处理从begin到time(不含)的主力合约=lastContract
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
		cout << "code = " << code << "info文件读取错误\n";
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
			cout << "main " << contract << "\t" << time << "\t处理成功！\n";
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
		cout << "code = " << code << ", info配置文件读取失败\n";
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
			cout << "contract " << contract << "\t" << time << "\t处理成功！\n";
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

//返回下一天
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

//删除指定文件夹路径下文件date日期之后数据
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