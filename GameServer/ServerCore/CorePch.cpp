#include "pch.h"
#include "CorePch.h"

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>


void save(const std::string& fileName ,const std::string& TargetfileName)
{
	// fileName �б� ���� ����
	std::ifstream in{ fileName }; //���� ��ü�̱� ������ �Ҹ��ڰ� ȣ��ȴ�,  //RAII


	// ������ ������ ���ٿ� ���� ���� ����
	std::ofstream out{ TargetfileName,std::ios::app };

	// ����ð��� ���Ͽ� ����Ѵ�



	auto now = std::chrono::system_clock::now();         // time_point
	auto utc = std::chrono::system_clock::to_time_t(now);   // utc �ð����� ��ȯ
	auto lt = localtime(&utc);                        // �����ð����� ��ȯ
	auto old = out.imbue(std::locale("ko_KR"));      // ���� ��ȯ
	//�ѱ� �������� ��¥ ��� ����
	using namespace std::literals::chrono_literals;

	out << "\n" << "\n";
	out << "==================================================" << "\n";
	out << fileName << std::put_time(lt, ", ����ð�: %x %A %X") << "\n";
	out << "==================================================" << "\n";
	out.imbue(old);                           //���� ����

	// fileName�� �ִ� ��� ������ �о� ������ ���Ͽ� ����
	// (STL�� �ڷᱸ���� �˰����� �̿��Ͽ� �����Ѵ�)
	std::vector<char> v{ std::istreambuf_iterator<char>{in},{} }; //{in}, {} �ڷ��� ����, �ڷ��� �� //istreambuf_iterator ��ĭ���� ���
	std::copy(v.begin(), v.end(), std::ostream_iterator<char>{ out });
}