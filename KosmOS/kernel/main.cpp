// 사용하려면 Clang에게 알려야한다. -> source buildenv.sh
#include <cstdint>
// 부트로더에서 픽셀을 그리는 것은 이미 성공
// so, 커널에서 똑같이 그리기 -> 부트로더에서 커널로 렌더링 정보 넘기기

// 파라미터로 2개의 64 비트 정수 추가
extern "C" void KernelMain(uint64_t frame_buffer_base,
							uint64_t frame_buffer_size){
	// C++ 스러운 타입 캐스팅 // C 언어 (uint8_t*) frame_buffer_base와 유사
	// "정수와 포인터의 전환이기에, 신중해야 한다"를 쉽게 알 수 있다.
	uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(frame_buffer_base);
	for(uint64_t i=0; i<frame_buffer_size; ++i){
		frame_buffer[i] = i%256;
	}
	while (1) __asm__("hlt");
}