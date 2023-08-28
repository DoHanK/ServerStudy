#include "pch.h"
#include "CorePch.h"

#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>


void save(const std::string& fileName ,const std::string& TargetfileName)
{
	// fileName 읽기 모드로 연다
	std::ifstream in{ fileName }; //지역 객체이기 때문에 소멸자가 호출된다,  //RAII


	// 저장할 파일을 덧붙여 쓰기 모드로 연다
	std::ofstream out{ TargetfileName,std::ios::app };

	// 저장시간을 파일에 기록한다



	auto now = std::chrono::system_clock::now();         // time_point
	auto utc = std::chrono::system_clock::to_time_t(now);   // utc 시간으로 변환
	auto lt = localtime(&utc);                        // 지역시간으로 변환
	auto old = out.imbue(std::locale("ko_KR"));      // 로켈 변환
	//한국 형식으로 날짜 출력 가능
	using namespace std::literals::chrono_literals;

	out << "\n" << "\n";
	out << "==================================================" << "\n";
	out << fileName << std::put_time(lt, ", 저장시간: %x %A %X") << "\n";
	out << "==================================================" << "\n";
	out.imbue(old);                           //로켈 복구

	// fileName에 있는 모든 내용을 읽어 저장할 파일에 쓴다
	// (STL의 자료구조와 알고리즘을 이용하여 저장한다)
	std::vector<char> v{ std::istreambuf_iterator<char>{in},{} }; //{in}, {} 자료의 시작, 자료의 끝 //istreambuf_iterator 빈칸까지 출력
	std::copy(v.begin(), v.end(), std::ostream_iterator<char>{ out });
}