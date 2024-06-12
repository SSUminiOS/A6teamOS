// 10개 Library
#include  <Uefi.h>
#include  <Library/UefiLib.h>
// git checkout day03a 추가 start.1
// 아마 day02에서 진행했던 내용일 가능성이 높음
// 라이브러리, 구조체, 함수 선언 및 정의부
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/PrintLib.h>
// git checkout day03b 추가 start.0
#include  <Library/MemoryAllocationLib.h>
// git checkout day03b 추가 end.0
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Protocol/DiskIo2.h>
#include  <Protocol/BlockIo.h>
#include  <Guid/FileInfo.h>
// git checkout day04b 추가 start
#include  "frame_buffer_config.hpp"
// git checkout day04b 추가 end
// git checkout day04d 추가 start
#include  <Library/BaseMemoryLib.h>
#include  "elf.hpp"
// git checkout day04b 추가 end

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
  // git checkout day03d 추가 start
  EFI_STATUS status;
  // git checkout day03d 추가 end
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  // git checkout day03d 추가 start
  status = file->Write(file, &len, header);
  if (EFI_ERROR(status)) {
    return status;
  }
  // git checkout day03d 추가 end
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
	// git checkout day03d 추가 start
    status = file->Write(file, &len, buf);
    if (EFI_ERROR(status)) {
      return status;
		}
	}
	// git checkout day03d 추가 end
  return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_STATUS status;
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  // git checkout day03d 추가 start
  status = gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  status = gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }
  return fs->OpenVolume(fs, root);
}
// git checkout day03d 추가 end

// git checkout day03a 추가 end.1

// git checkout day03b 추가 start.1
EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
                   EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  EFI_STATUS status;					   
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;

  // git checkout day03d 추가 start
  status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &num_gop_handles,
      &gop_handles);
  if (EFI_ERROR(status)) {
    return status;
  }

  status = gBS->OpenProtocol(
      gop_handles[0],
      &gEfiGraphicsOutputProtocolGuid,
      (VOID**)gop,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }
  
  FreePool(gop_handles);

  return EFI_SUCCESS;
}
// git checkout day03d 추가 end

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
// git checkout day03b 추가 end.1

// git checkout day03d 추가 start.2
/* hlt를 안하고 return을 하면, 부트로더가 구동처리 이어감
-> 화면이 갱신돼 에러 메세지가 사라진다. -> 무한루프로 실행을 멈춘이유*/ 
void Halt(void) {
  while (1) __asm__("hlt");
}

// git checkout day04d 추가 start
// #@@range_begin(calc_addr_func)
void CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first, UINT64* last) {
  Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  *first = MAX_UINT64;
  *last = 0;
  for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type != PT_LOAD) continue;
    *first = MIN(*first, phdr[i].p_vaddr);
    *last = MAX(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
  }
}
// #@@range_end(calc_addr_func)

// #@@range_begin(copy_segm_func)
void CopyLoadSegments(Elf64_Ehdr* ehdr) {
  Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type != PT_LOAD) continue;

    UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
    CopyMem((VOID*)phdr[i].p_vaddr, (VOID*)segm_in_file, phdr[i].p_filesz);

    UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
    SetMem((VOID*)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
  }
}
// #@@range_end(copy_segm_func)
// git checkout day04d 추가 end

// git checkout day03d 추가 end.2

EFI_STATUS EFIAPI UefiMain( EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
	EFI_STATUS status;
	Print(L"Hello, KosmOS!\n");
  
    CHAR8 memmap_buf[4096 * 4];
	struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
	
	// git checkout day03d 추가 start
	status = GetMemoryMap(&memmap);
	if (EFI_ERROR(status)) {
		Print(L"failed to get memory map: %r\n", status);
		Halt();
	}
	// git checkout day03d 추가 end
	EFI_FILE_PROTOCOL* root_dir;
	status = OpenRootDir(image_handle, &root_dir);
	if (EFI_ERROR(status)) {
		Print(L"failed to open root directory: %r\n", status);
		Halt();
	}
	// git checkout day03d 추가 start
	EFI_FILE_PROTOCOL* memmap_file;
	status = root_dir->Open(
		root_dir, &memmap_file, L"\\memmap",
		EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
	if (EFI_ERROR(status)) {
		Print(L"failed to open file '\\memmap': %r\n", status);
		Print(L"Ignored.\n");
	} else {
	status = SaveMemoryMap(&memmap, memmap_file);
	if (EFI_ERROR(status)) {
		Print(L"failed to save memory map: %r\n", status);
		Halt();
	}
	status = memmap_file->Close(memmap_file);
		if (EFI_ERROR(status)) {
			Print(L"failed to close memory map: %r\n", status);
			Halt();
		}
	}
	
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	status = OpenGOP(image_handle, &gop);
	if (EFI_ERROR(status)) {
		Print(L"failed to open GOP: %r\n", status);
		Halt();
	}
	
	Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
		gop->Mode->Info->PixelsPerScanLine);
	Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
		gop->Mode->FrameBufferBase,
		gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
		gop->Mode->FrameBufferSize);
	// git checkout day03d 추가 end
	
	UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
	for (UINTN i=0; i<gop->Mode->FrameBufferSize; i++){
		frame_buffer[i]=255; // 8비트 전체가 1 0xff -> 흰색
	}
	// git checkout osbook day03b 추가 end.2
	// #@@range_end(gop)

	// #@@range_begin(read_kernel)
	// kernel.elf를 읽기 전형으로 열기
	EFI_FILE_PROTOCOL* kernel_file;
	// git checkout day03d 추가 start
	status = root_dir->Open(
		root_dir, &kernel_file, L"\\kernel.elf",
		EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		Print(L"failed to open file '\\kernel.elf': %r\n", status);
		Halt();
	}
	// git checkout day03d 추가 end
	// 미리 메모리 잡아 놓기
	/* 왜 더 크게 잡는가 (?) -> 구조체에 FileName 부분 배열 요소 개수 정의 X,
	so, FileName 부분 크기 미리 잡을 필요 있음*/

	UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
	UINT8 file_info_buffer[file_info_size];
	status = kernel_file->GetInfo(
	// 파일 크기 계산 후, 생성한 메모리에 담기
		kernel_file, &gEfiFileInfoGuid,
		&file_info_size, file_info_buffer);
	if (EFI_ERROR(status)) {
		Print(L"failed to get file information: %r\n", status);
		Halt();
	}
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
	// git checkout day03d 추가 start.1
	/* error 해결 방법론: 1. 복구, 2. 중단 -> 일괄로 중단 선택
	요구된 크기의 메모리 영역을 확보하지 못한 경우
	-> EFI_OUT_OF_RESOURCES 반환 -> 커널 구동 처리 중단
	*/
	
	VOID* kernel_buffer;
	status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(L"failed to allocate pool: %r\n", status);
		Halt();
	}
	status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
		if (EFI_ERROR(status)) {
		Print(L"error: %r", status);
	Halt();
	}
	// #@@range_end(read_kernel)
	
	// #@@range_begin(alloc_pages)
	Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;
	UINT64 kernel_first_addr, kernel_last_addr;
	CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
	
	UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
	status = gBS->AllocatePages(AllocateAddress, EfiLoaderData,
								num_pages, &kernel_first_addr);
	if (EFI_ERROR(status)) {
		Print(L"failed to allocate pages: %r\n", status);
		Halt();
	}
	// #@@range_end(alloc_pages)
	
	// #@@range_begin(copy_segments)
	CopyLoadSegments(kernel_ehdr);
	Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);
	
	status = gBS->FreePool(kernel_buffer);
	if (EFI_ERROR(status)) {
		Print(L"failed to free pool: %r\n", status);
		Halt();
	}
  // #@@range_end(copy_segments)
	/* from modified loader start
	EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
	status = gBS->AllocatePages(
		AllocateAddress, EfiLoaderData,
		(kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);
	if (EFI_ERROR(status)){
		Print(L"failed to allocate pages: %r", status);
		Halt();
	}
	// git checkout day03d 추가 end.1
	// 공간 확보 완료 했으니, 파일 읽어 들이기
	status = kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
	if (EFI_ERROR(status)) {
		Print(L"error: %r", status);
		Halt();
	}
	Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size); 
	from modified loader end */
	// 커널 구동 시키는 일만 남았다 (?)
	// 아닙니다. UEFI BIOS의 부트 서비스 중지 필요 (OS에 방해됨)

	/* ExitBootServices()는 최신 메모리 맵의 맵 키 요구
	지정된 맵 키가 최신의 메모리 맵과 연결된 맵 키가 아닌 경우 실행 실패

	실패시 다시 메모리 맵을 얻고 해당 맵 키를 사용해 재실행
	초기 메모리 맵 취득 -> 함수 호출 사이 여러 기능 사용 -> 첫 시도 반드시 실패
	두번째 실행 -> 실패 시 중대한 오류 이므로 무한루프 처리로 정지 (?)
	*/
	// #@@range_begin(exit_bs)
	status = gBS->ExitBootServices(image_handle, memmap.map_key);
	if (EFI_ERROR(status)){
		status = GetMemoryMap(&memmap);
		if (EFI_ERROR(status)){
			Print(L"failed to get memmory map: %r\n", status);
			//while(1);
			Halt();
		}
		status = gBS-> ExitBootServices(image_handle, memmap.map_key);
		if (EFI_ERROR(status)) {
			Print(L"Could not exit boot service: %r\n", status);
			//while(1);
			Halt();
		}
	}
	// #@@range_end(exit_bs)
	
	// #@@range_begin(call_kernel)
	/* 커널 가동하는 부분
	엔트리 포인트가 놓인 위치를 계산해서 엔트리 포인트 호출
	엔트리 포인트: C언어 에서는 main()
	현재 엔트리 포인트: KernelMain()
	So, KernelMain()이 있는지 특정, 호출하는 것이 부트로더의 큰 목적

	EFL 형식의 사양에 따르면, 64비트용 ELF 엔트리 포인트 어드레스 위치:
	오프셋 24 바이트 위치에서 8파이트 정수로 작성 */
	
	// git checkout day04d 추가 start
	UINT64 entry_addr = *(UINT64*)(kernel_first_addr+24);
	// git checkout day04d 추가 end
	struct FrameBufferConfig config = {
		(UINT8*)gop->Mode->FrameBufferBase,
		gop->Mode->Info->PixelsPerScanLine,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		0
	};
	switch (gop->Mode->Info->PixelFormat) {
	case PixelRedGreenBlueReserved8BitPerColor:
		config.pixel_format = kPixelRGBResv8BitPerColor;
		break;
	case PixelBlueGreenRedReserved8BitPerColor:
		config.pixel_format = kPixelBGRResv8BitPerColor;
		break;
	default:
		Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
		Halt();
	}
	
	//typedef void EntryPointType(const struct FrameBufferConfig*);
	typedef void __attribute__((sysv_abi)) EntryPointType(const struct FrameBufferConfig*);
	EntryPointType* entry_point = (EntryPointType*)entry_addr;
	entry_point(&config);
	// 포인터를 함수로 정의 하기 위한 준비
	// Parameter, return 모두 void인 함수
	
	// git checkout day03c 추가 start.1
	// typedef void EntryPointType(void);
	// typedef void EntryPointType(UINT64, UINT64);
	// git checkout day04b 주석 start
	// typedef void __attribute__((sysv_abi)) EntryPointType(UINT64, UINT64);
	// EntryPointType* entry_point = (EntryPointType*)entry_addr;
	
	// So, 함수로 호출 가능
	//entry_point(gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize); 
	// git checkout day03c 추가 start.1
	// git checkout day04b 주석 end
	// #@@range_end(call_kernel)
	
	Print(L"All done\n");
	
	while (1);
	return EFI_SUCCESS;
}
