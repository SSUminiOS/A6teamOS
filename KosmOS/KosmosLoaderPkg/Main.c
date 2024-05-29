// 10개 Library
#include  <Uefi.h>
#include  <Library/UefiLib.h>
// 아마 day02에서 진행했던 내용일 가능성이 높음
// 라이브러리, 구조체, 함수 선언 및 정의부
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/PrintLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Protocol/DiskIo2.h>
#include  <Protocol/BlockIo.h>
#include  <Guid/FileInfo.h>

struct MemoryMap {
  UINTN buffer_size;
  VOID* buffer;
  UINTN map_size;
  UINTN map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;
};

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap(
      &map->map_size,
      (EFI_MEMORY_DESCRIPTOR*)map->buffer,
      &map->map_key,
      &map->descriptor_size,
      &map->descriptor_version);
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}

EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  file->Write(file, &len, header);

  Print(L"map->buffer = %08lx, map->map_size = %08lx\n",
      map->buffer, map->map_size);

  EFI_PHYSICAL_ADDRESS iter;
  int i;
  for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
       iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
       iter += map->descriptor_size, i++) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
    len = AsciiSPrint(
        buf, sizeof(buf),
        "%u, %x, %-ls, %08lx, %lx, %lx\n",
        i, desc->Type, GetMemoryTypeUnicode(desc->Type),
        desc->PhysicalStart, desc->NumberOfPages,
        desc->Attribute & 0xffffflu);
    file->Write(file, &len, buf);
  }
  return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  fs->OpenVolume(fs, root);

  return EFI_SUCCESS;
}

EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
                   EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;
  gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &num_gop_handles,
      &gop_handles);

  gBS->OpenProtocol(
      gop_handles[0],
      &gEfiGraphicsOutputProtocolGuid,
      (VOID**)gop,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  FreePool(gop_handles);

  return EFI_SUCCESS;
}

const CHAR16* GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
  switch (fmt) {
    case PixelRedGreenBlueReserved8BitPerColor:
      return L"PixelRedGreenBlueReserved8BitPerColor";
    case PixelBlueGreenRedReserved8BitPerColor:
      return L"PixelBlueGreenRedReserved8BitPerColor";
    case PixelBitMask:
      return L"PixelBitMask";
    case PixelBltOnly:
      return L"PixelBltOnly";
    case PixelFormatMax:
      return L"PixelFormatMax";
    default:
      return L"InvalidPixelFormat";
  }
}

EFI_STATUS EFIAPI UefiMain( EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
	Print(L"Hello, KosmOS!\n");
  
    CHAR8 memmap_buf[4096 * 4];
	struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
	GetMemoryMap(&memmap);
	
	EFI_FILE_PROTOCOL* root_dir;
	OpenRootDir(image_handle, &root_dir);

	EFI_FILE_PROTOCOL* memmap_file;
	root_dir->Open(
      root_dir, &memmap_file, L"\\memmap",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

	SaveMemoryMap(&memmap, memmap_file);
	memmap_file->Close(memmap_file);
	
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	OpenGOP(image_handle, &gop);
	Print(L"Resoultion: %ux%u, Pixel Format: %s, %u pixels/line\n",
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
		gop->Mode->Info->PixelsPerScanLine);
	Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
		gop->Mode->FrameBufferBase,
		gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
		gop->Mode->FrameBufferSize);
		
	UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
	for (UINTN i=0; i<gop->Mode->FrameBufferSize; i++){
		frame_buffer[i]=255; // 8비트 전체가 1 0xff -> 흰색
	}
	
	// kernel.elf를 읽기 전형으로 열기
	EFI_FILE_PROTOCOL* kernel_file;
	root_dir->Open(
		root_dir, &kernel_file, L"\\kernel.elf",
		EFI_FILE_MODE_READ, 0);

	// 미리 메모리 잡아 놓기
	/* 왜 더 크게 잡는가 (?) -> 구조체에 FileName 부분 배열 요소 개수 정의 X,
	so, FileName 부분 크기 미리 잡을 필요 있음*/

	UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
	UINT8 file_info_buffer[file_info_size];

	// 파일 크기 계산 후, 생성한 메모리에 담기
	kernel_file->GetInfo(
		kernel_file, &gEfiFileInfoGuid,
		&file_info_size, file_info_buffer);

	// 타입 캐스팅
	EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
	UINTN kernel_file_size = file_info->FileSize;

	/* 이제 커널의 크기를 알았기 때문에, 메모리 확보
	1. 메모리 확보 방법
		1-1. 어디라도 좋으니, 비어있는 공간에서 확보: AllocateAnyPages
		1-2. 지정한 어드레스 이후에 비어있는 공간에서 확보: AllocateMaxAddress
		1-3. 지정한 주소에서 확보: AllocateAddress
		커널은 0x100000번지에 배치를 전제로 만들었다. -> ld.lld 옵션
		so, 다른 위치에 배치시키면 정상 작동 X
		
	2. 확보할 메모리 영역 유형
	3. 크기
	4. 확보한 메모리 공간의 어드레스를 저장하기 위한 변수 지정
		포인터 넘기는 이유: AllocateAnyPages, AllocateMaxAddress의 경우 값 저장 따로 필요
		but now I don't need it -> 변숫값이 변뀌는 경우가 없기 때문
		return 값이 성공 실패인 상황에서 다른 파라미터 전달: 포인터로 구현 */

	/* 페이지 수 = (kernel_file_size + 0xfff) / 0x1000
	0xfff 더해주는 이유: 페이지수를 올림 하기 위해 -> 잘려서 누락되는 것 방지 */
	EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
	gBS->AllocatePages(
		AllocateAddress, EfiLoaderData,
		(kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);

	// 공간 확보 완료 했으니, 파일 읽어 들이기
	kernel_file -> Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
	Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size);

	// 커널 구동 시키는 일만 남았다 (?)
	// 아닙니다. UEFI BIOS의 부트 서비스 중지 필요 (OS에 방해됨)

	/* ExitBootServices()는 최신 메모리 맵의 맵 키 요구
	지정된 맵 키가 최신의 메모리 맵과 연결된 맵 키가 아닌 경우 실행 실패

	실패시 다시 메모리 맵을 얻고 해당 맵 키를 사용해 재실행
	초기 메모리 맵 취득 -> 함수 호출 사이 여러 기능 사용 -> 첫 시도 반드시 실패
	두번째 실행 -> 실패 시 중대한 오류 이므로 무한루프 처리로 정지 (?)
	*/

	EFI_STATUS status;
	status = gBS->ExitBootServices(image_handle, memmap.map_key);
	if (EFI_ERROR(status)){
		status = GetMemoryMap(&memmap);
		if (EFI_ERROR(status)){
			Print(L"failed to get memmory map: %r\n", status);
			while(1);
		}
		status = gBS-> ExitBootServices(image_handle, memmap.map_key);
		if (EFI_ERROR(status)) {
			Print(L"Could not exit boot service: %r\n", status);
			while(1);
		}
	}

	/* 커널 가동하는 부분
	엔트리 포인트가 놓인 위치를 계산해서 엔트리 포인트 호출
	엔트리 포인트: C언어 에서는 main()
	현재 엔트리 포인트: KernelMain()
	So, KernelMain()이 있는지 특정, 호출하는 것이 부트로더의 큰 목적

	EFL 형식의 사양에 따르면, 64비트용 ELF 엔트리 포인트 어드레스 위치:
	오프셋 24 바이트 위치에서 8파이트 정수로 작성 */
	UINT64 entry_addr = *(UINT64*)(kernel_base_addr+24);

	// 포인터를 함수로 정의 하기 위한 준비
	// Parameter, return 모두 void인 함수
	
	// typedef void EntryPointType(void);
	// typedef void EntryPointType(UINT64, UINT64);
	typedef void __attribute__((sysv_abi)) EntryPointType(UINT64, UINT64);
	EntryPointType* entry_point = (EntryPointType*)entry_addr;
	
	// So, 함수로 호출 가능
	entry_point(gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize); 
	
	Print(L"All done\n");
	
	while (1);
	return EFI_SUCCESS;
}
