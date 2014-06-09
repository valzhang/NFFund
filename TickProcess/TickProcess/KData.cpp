#include "KData.h"

//std::ofstream outTest("C:\\Users\\ValZhang\\Desktop\\commodity\\CFFE\\test.txt");

KData::KData()
{
	this->period_num = 0;
	this->transfer_time_begin = NULL;
	this->transfer_time_end = NULL;
	this->k_line_num = 0;
	this->k_gap = 0;
	this->transfer_minute = 0;

	this->data_num = 15;
	this->k_data = new double**[this->data_num + 1];

		this->k_1_mean = 0;
		this->k_5_mean = 0;

		this->k_index = new int[this->k_line_num];
		for ( int i = 0; i < this->k_line_num; i++ ){
			this->k_index[i] = 0;
			}

}

KData::~KData()
{

}

void KData::DeleteData()
{		
	delete []this->transfer_time_begin;
	delete []this->transfer_time_end;
	delete []this->k_gap;

	for ( int i = 0; i < this->data_num; i++ ){
		if ( this->k_data[i] != NULL ){

			delete [] (this->k_data[i]);
		}
	}
	delete [] (this->k_data);
}

//设置交易时间段个数
void KData::SetTransferPeriod( int n )
{
	this->period_num = n;
	this->transfer_time_begin = new int[n + 1];
	this->transfer_time_end = new int[n + 1];
	memset( this->transfer_time_begin, 0, sizeof(int) );
	memset( this->transfer_time_end, 0, sizeof(int) );
}

//设置第n个交易时间段
bool KData::SetTransferTime( int n, int begin, int end )
{
	if ( n > 1 && this->transfer_time_end[n - 2] > begin ){
		return false;
	}
	this->transfer_time_begin[n - 1] = begin;
	this->transfer_time_end[n - 1] = end;
	this->transfer_minute += ( end - begin );
	return true;
}

//设置K线个数
void KData::SetKLineNum( int n )
{
	this->k_line_num = n;
	this->k_gap = new int[n + 1];
	for ( int i = 0; i < this->data_num; i++ ){
		this->k_data[i] = new double*[n + this->period_num + 1];
	}
}

//获取K线个数
int KData::GetLineNum()
{
	return this->k_line_num;
}

//设置第n个K线参数
void KData::SetKLine( int n, int gap )
{
	this->k_gap[n - 1] = gap;
}

//获得第n个K线参数
int KData::GetKLine( int n )
{
	return this->k_gap[n - 1];
}

//设置参数完毕，准备输入数据
bool KData::GetReady()
{	
	if ( this->period_num == 0 || this->transfer_time_begin == NULL || this->transfer_time_end == NULL ||
		this->k_line_num == 0 || this->k_gap == NULL || this->transfer_minute == 0 || this->data_num == 0 ||
		this->k_data == NULL ){
			return false;
	}
	int num;
	for ( int i = 0; i < this->data_num; i++ ){
		if ( this->k_data[i] == NULL ){
			return false;
		}
		for ( int j = 0; j < this->k_line_num; j++ ){
			if ( this->k_gap[j] < 0 ){
				num =  2;
			}else{
				num =  (int)(this->transfer_minute / this->k_gap[j]) + this->period_num ;
			}
			this->k_data[i][j] = new double[num + 1];
			for ( int k = 0; k < num; k++ ){
				this->k_data[i][j][k] = 0;
			}
		}
	}
	return true;
}

//删除所有Tick数据
void KData::ClearData()
{
	for ( int i = 0; i < this->data_num; i++ ){
		for ( int j = 0; j < this->k_line_num; j++ ){
			for ( int k = 0; k <= this->CalMinuteIndex( this->transfer_time_end[ this->period_num - 1 ], this->k_gap[j] ); k++ ){
				this->k_data[i][j][k] = 0;
			}
		}
	}
	for ( int i = 0; i < this->k_line_num; i++ ){
		this->k_index[i] = 0;
	}
}

//添加一个Tick数据
//生成K线的逻辑
bool KData::AddData( std::string contract, int date, double *data )
{
	if ( this->k_gap == NULL){
		return false;
	}
	bool flag = false;
	for ( int i = 0; i < this->period_num; i++ ){
		if ( data[0]/100 >= this->transfer_time_begin[i] && data[0]/100 <= this->transfer_time_end[i] ){
			flag = true;
			break;
		}
	}
	if ( !flag ){
		//std::cout << data[0] << std::endl;
		return false;
	}
	for ( int i = 0; i < this->k_line_num; i++ ){
		//对每种K线
		int num = this->CalMinuteIndex( (int)(data[0]/100), this->k_gap[i] );
		this->k_index[i] = num;
		if ( this->k_data[0][i][num] == 0 ){	//如果是第一个Tick数据
			this->k_data[0][i][num] = date;
			this->k_data[1][i][num] = this->NearMinute( (int)(data[0]/100), this->k_gap[i] );
			this->k_data[2][i][num] = data[1];
			this->k_data[3][i][num] = data[1];
			this->k_data[4][i][num] = data[1];
			this->k_data[5][i][num] = data[1];
			this->k_data[6][i][num] = data[4];
			this->k_data[7][i][num] = data[14];
			this->k_data[8][i][num] = data[9];
			this->k_data[9][i][num] = data[19];
			this->k_data[10][i][num] = data[2];
			this->k_data[11][i][num] = data[3];
			this->k_data[12][i][num] = data[24];
			this->k_data[13][i][num] = data[25];
			this->k_data[14][i][num] = data[26];
		}else{								
			this->k_data[3][i][num] = data[1];
			this->k_data[4][i][num] = data[1] > this->k_data[4][i][num] ? data[1] : this->k_data[4][i][num];
			this->k_data[5][i][num] = data[1] < this->k_data[5][i][num] ? data[1] : this->k_data[5][i][num];
//			if ( this->k_data[6][i][num] < data[4] ){		//买一
				this->k_data[6][i][num] = data[4];
				this->k_data[7][i][num] = data[14];
//			}
//			if ( this->k_data[8][i][num] > data[9] ){		//卖一
				this->k_data[8][i][num] = data[9];
				this->k_data[9][i][num] = data[19];
//			}
			this->k_data[10][i][num] += data[2];
			this->k_data[11][i][num] += data[3];
			this->k_data[12][i][num] = data[24];
			this->k_data[13][i][num] = data[25];
			this->k_data[14][i][num] = data[26];
		}
		if ( this->k_gap[i] == 1 ){
			if ( num < 10 ){
				double sum = 0;
				for ( int j = 0; j <= num; j++ ){
					if ( this->k_data[3][i][j] == 0 ){
						sum += this->k_data[3][i][num];
					}else{
						sum += this->k_data[3][i][j];
					}
				}
				this->k_1_mean = sum / (num + 1);
			}else{
				double sum = 0;
				for ( int j = num; j > num-10; j-- ){
					if ( this->k_data[3][i][j] == 0 ){
						sum += this->k_data[3][i][num];
					}else{
						sum += this->k_data[3][i][j];
					}
				}
				this->k_1_mean = sum / 10;
			}
		}
		if ( this->k_gap[i] == 5 ){
			if ( num < 6 ){
				double sum = 0;
				for ( int j = 0; j <= num; j++ ){
					if ( this->k_data[3][i][j] == 0 ){
						sum += this->k_data[3][i][num];
					}else{
						sum += this->k_data[3][i][j];
					}
				}
				this->k_5_mean = sum / (num + 1);
			}else{
				double sum = 0;
				for ( int j = num; j > num - 6; j-- ){
					if ( this->k_data[3][i][j] == 0 ){
						sum += this->k_data[3][i][num];
					}else{
						sum += this->k_data[3][i][j];
					}
				}
				this->k_5_mean = sum / 6;
			}
		}
	}
	
//		outTest<< "time = " << data[0] << " data[1] == " << data[1] << " 一分钟均值 = " << this->k_1_mean << " 五分钟均值 = " << this->k_5_mean;
//	if ( this->k_5_mean < this->k_1_mean && data[1] > this->k_1_mean ){
//		outTest << "——BUY——" << contract << "——BUY——" ;
///		std::cout << "Strategy 1 Signal——>>>>>>> " << contract << " " << data[0] << " BUY<<<<<<<<<——Strategy 1 Signal\n";
//	}
//	outTest << '\n';
//	std::cout << "add a data\n";
	return true;
}

	
//计算最近的时间开始标记分钟
int KData::NearMinute( int minute, int gap )
{
	if ( gap < 0 ){
		return 900;
	}
		for ( int i = 0; i < this->period_num; i++ ){
		if ( minute == this->transfer_time_end[i] ){
			minute--;
			if ( minute % 100 > 59 ){
				minute -= 40;
			}
		}
	}
	for ( int i = 0; i < this->period_num; i++ ){
		if ( this->transfer_time_end[i] > minute ){
			int resi = this->CalGapMin( this->transfer_time_begin[i], minute ) % gap;
			if ( minute % 100 < resi ){
				minute -= 40;
			}//1001 961
			return ( minute - resi );
		}
	}
}

//计算时间编号
int KData::CalMinuteIndex( int minute, int gap )
{
	if ( gap < 0 ){
		return 0;
	}
	for ( int i = 0; i < this->period_num; i++ ){
		if ( minute == this->transfer_time_end[i] ){
			minute--;
			if ( minute % 100 > 59 ){
				minute -= 40;
			}
		}
	}
	int index = 0;
	for ( int i = 0; i < this->period_num; i++ ){
		if ( this->transfer_time_end[i] > minute ){
			index += (int) (this->CalGapMin( this->transfer_time_begin[i], minute )) / gap;	//从0开始编号，最后一次不需要+1
			break;
		}else{
			index += (int) (this->CalGapMin( this->transfer_time_begin[i], this->transfer_time_end[i] ) - 1) / gap + 1;
		}
	}
	return index;
}

bool KData::IsTransferTime( int time )
{
	bool flag = false;
	for ( int i = 0; i < this->period_num; i++ ){
		if ( time >= this->transfer_time_begin[i] && time < this->transfer_time_end[i] ){
			flag = true;
			break;
		}
	}
	return flag;
}

//计算两个时间之间的分钟数
int KData::CalGapMin( int begin, int end )
{
	int b_h = (int)(begin/100);
	int b_m = begin % 100;
	int e_h = (int)(end/100);
	int e_m = end % 100;
	if ( e_m < b_m ){
		e_m += 60;
		e_h--;
	}
	return (e_h - b_h) * 60 + (e_m - b_m);
}

//补全空值数据
void KData::FillData()
{
	for ( int i = 0; i < this->k_line_num; i++){
		if ( this->k_data[0][i][0] == 0 )
		{
			int index = 0;
			do{
				index++;
			}while ( this->k_data[0][i][index] == 0 );
			this->k_data[0][i][0] = this->k_data[0][i][index];
			this->k_data[1][i][0] = this->transfer_time_begin[0];
			this->k_data[2][i][0] = this->k_data[2][i][index];
			this->k_data[3][i][0] = this->k_data[2][i][index];
			this->k_data[4][i][0] = this->k_data[2][i][index];
			this->k_data[5][i][0] = this->k_data[2][i][index];
			this->k_data[6][i][0] = 0;
			this->k_data[7][i][0] = 0;
			this->k_data[8][i][0] = 0;
			this->k_data[9][i][0] = 0;
			this->k_data[10][i][0] = 0;
			this->k_data[11][i][0] = 0;
			this->k_data[12][i][0] = this->k_data[12][i][index] - this->k_data[10][i][index];
			this->k_data[13][i][0] = this->k_data[13][i][index] - this->k_data[11][i][index];
			this->k_data[14][i][0] = this->k_data[14][i][index];
		}
		for ( int j = 1; j <= this->CalMinuteIndex( this->transfer_time_end[ this->period_num - 1 ], this->k_gap[i] ); j++ ){
			if ( this->k_data[0][i][j] == 0 ){
				this->k_data[0][i][j] = this->k_data[0][i][j - 1];
				int m = this->k_data[1][i][j - 1] + 100 * (int)(this->k_gap[i] / 60) + (this->k_gap[i] % 60);
				if ( m % 100 > 59 ){
					m += 40;
				}
				if ( !this->IsTransferTime( m ) ){		//若m不是交易时间
					for ( int ii = 1; ii < this->period_num; ii++ ){
						if ( m < this->transfer_time_begin[ii] ){
							m = this->transfer_time_begin[ii];
							break;
						}
					}
				}
				this->k_data[1][i][j] = m;
				this->k_data[2][i][j] = this->k_data[3][i][j - 1];
				this->k_data[3][i][j] = this->k_data[3][i][j - 1];
				this->k_data[4][i][j] = this->k_data[3][i][j - 1];
				this->k_data[5][i][j] = this->k_data[3][i][j - 1];
				this->k_data[12][i][j] = this->k_data[12][i][j - 1];
				this->k_data[13][i][j] = this->k_data[13][i][j - 1];
				this->k_data[14][i][j] = this->k_data[14][i][j - 1];
			}
		}
	}
}

//返回下一分钟时间值
int KData::NextMinute( int minute )
{
	if ( minute % 100 == 59 ){
		minute += 40;
	}
	minute++;
	return minute;
}

//返回上一分钟时间值
int KData::LastMinute( int minute )
{
	if ( minute % 100 == 0 ){
		minute -= 40;
	}
	minute--;
	return minute;
}

//输出数据到文件
void KData::PrintData( std::string path, std::string contract, char* chTime )
{
	for ( int i = 0; i < this->k_line_num; i++ ){
		std::string print_path = path;
		print_path.append( contract );
		print_path.append( "_" );
		print_path.append( chTime );
		print_path.append( "_" );		
		if ( this->k_gap[i] < 0 ){
			print_path.append( "day" );
		}else{
			char kCH[3];
			itoa( this->k_gap[i], kCH, 10 );
			print_path.append( kCH );
		}		
		print_path.append( ".txt" );
		std::ofstream out( print_path );
		for ( int j = 0; j <= this->CalMinuteIndex( this->transfer_time_end[ this->period_num - 1 ], this->k_gap[i] ); j++ ){
			bool flag = false;
			for ( int ii = 0; ii < this->period_num; ii++ ){
				if ( this->k_data[1][i][j] >= this->transfer_time_begin[ii] && this->k_data[1][i][j] <= this->transfer_time_end[ii] ){
					flag = true;
					break;
				}
			}
			if ( !flag ){
				continue;
			}
/*			out << (int)this->k_data[0][i][j] << '\t'
				<< (int)this->k_data[1][i][j] << '\t'
				<< this->k_data[2][i][j] << '\t'
				<< this->k_data[3][i][j] << '\t'
				<< this->k_data[4][i][j] << '\t'
				<< this->k_data[5][i][j] << '\t'
				<< this->k_data[6][i][j] << '\t'
				<< (int)this->k_data[7][i][j] << '\t'
				<< this->k_data[8][i][j] << '\t'
				<< (int)this->k_data[9][i][j] << '\t'
				<< (long long)this->k_data[10][i][j] << '\t'
				<< this->k_data[11][i][j] << '\t'
				<< this->k_data[12][i][j] << '\t'
				<< this->k_data[13][i][j] << '\t'
				<< (long long)this->k_data[14][i][j] << '\t'
				<< contract.substr( contract.length() - 4, 4 ) << '\n';*/
			int yy = (int)(this->k_data[0][i][j]) / 10000;
			int mo = ((int)( this->k_data[0][i][j] / 100 )) % 100;
			int dd = ((int)( this->k_data[0][i][j] )) % 100;
			int hh = (int)(this->k_data[1][i][j]) / 100;
			int mi = ((int) this->k_data[1][i][j]) % 100;
			out << contract << '\t'
				<< yy << '-';
			if (mo < 10){
				out << 0;
			}
			out << mo << '-';
			if (dd < 10){
				out << 0;
			}
			out << dd << '\t';
			if (hh < 10){
				out << 0;
			}
			out << hh << ':';
			if (mi < 10){
				out << 0;
			}
			out << mi << ':';
			out << 0 << 0 << '\t';
			out << (int) this->k_data[0][i][j] << '\t'
				<< (int) this->k_data[1][i][j] << '\t'
				<< this->k_data[3][i][j] << '\t'
				<< this->k_data[2][i][j] << '\t'
				<< this->k_data[4][i][j] << '\t'
				<< this->k_data[5][i][j] << '\t'
				<< this->k_data[10][i][j] << '\t'
				<< this->k_data[11][i][j] << '\t'
				<< this->k_data[6][i][j] << '\t'
				<< this->k_data[7][i][j] << '\t'
				<< this->k_data[8][i][j] << '\t'
				<< this->k_data[9][i][j] << '\t'
				<< this->k_data[14][i][j] << '\n';

		}
		out.close();
	}
}

void KData::PrintLastData(std::ostream& out, std::string contract, int k)
{
	int index = this->k_index[k] - 1;
	int yy = (int)(this->k_data[0][k][index]) / 10000;
	int mo = ((int)( this->k_data[0][k][index] / 100 )) % 100;
	int dd = ((int)( this->k_data[0][k][index] )) % 100;
	int hh = (int)(this->k_data[1][k][index]) / 100;
			int mi = ((int) this->k_data[1][k][index]) % 100;
			out << contract << '\t'
				<< yy << '-';
			if (mo < 10){
				out << 0;
			}
			out << mo << '-';
			if (dd < 10){
				out << 0;
			}
			out << dd << '\t';
			if (hh < 10){
				out << 0;
			}
			out << hh << ':';
			if (mi < 10){
				out << 0;
			}
			out << mi << ':';
			out << 0 << 0 << '\t';
			out << (int) this->k_data[0][k][index] << '\t'
				<< (int) this->k_data[1][k][index] << '\t'
				<< this->k_data[3][k][index] << '\t'
				<< this->k_data[2][k][index] << '\t'
				<< this->k_data[4][k][index] << '\t'
				<< this->k_data[5][k][index] << '\t'
				<< this->k_data[10][k][index] << '\t'
				<< this->k_data[11][k][index] << '\t'
				<< this->k_data[6][k][index] << '\t'
				<< this->k_data[7][k][index] << '\t'
				<< this->k_data[8][k][index] << '\t'
				<< this->k_data[9][k][index] << '\t'
				<< this->k_data[14][k][index] << '\n';

}