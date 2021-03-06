// PakAnalyzer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include "ostream.hpp"

using namespace std;
using namespace text::csv;
namespace csv = text::csv;


// Helper class for safely use HANDLE
class SafeHandle {
private:
	HANDLE m_Handle;
public:
	SafeHandle(HANDLE h) :m_Handle(h) {};
	~SafeHandle() { CloseHandle(m_Handle); m_Handle = NULL; }
	operator HANDLE() { return m_Handle; }
};


enum ZIP_SIGNATURE {
	LocalFileHeader = 0x04034b50,
	DataDescriptor = 0x08074b50,
	CentralDirectoryFileHeader = 0x02014b50,
	EOCD = 0x06054b50
};
#pragma pack(push, 2)
__declspec(align(2)) struct _ZipLocalFileHeader
{
	WORD version;
	WORD bitflags;
	WORD comp_method;
	WORD lastModFileTime;
	WORD lastModFileDate;
	DWORD crc_32;
	DWORD comp_size;
	DWORD uncompr_size;
	WORD fname_len;
	WORD extra_field_len;
};
__declspec(align(2)) struct _ZipCDFHeader
{
	WORD version_madeby;
	WORD version_needed;
	WORD bitflags;
	WORD comp_method;
	WORD lastModFileTime;
	WORD lastModFileDate;
	DWORD crc_32;
	DWORD comp_size;
	DWORD uncompr_size;
	WORD fname_len;
	WORD extra_field_len;
	WORD fcomment_len;
	WORD disk_num_start;
	WORD internal_fattribute;
	DWORD external_fattribute;
	DWORD relative_offset;
};
__declspec(align(2)) struct _ZipEOCD	//Offset Bytes Description
{
	//0	4	End of central directory signature = 0x06054b50
	WORD numOfDisk;					//4	2	Number of this disk
	WORD diskCDStart;					//6	2	Disk where central directory starts
	WORD numOfCD;						//8	2	Number of central directory records on this disk
	WORD numTotalCD;					//10 2	Total number of central directory records
	DWORD sizeCD;						//12 4	Size of central directory(bytes)
	DWORD offsetCDStart;				//16 4	Offset of start of central directory, relative to start of archive
	WORD lenComment;					//20 2	Comment length(n)
										//22 n	Comment
};
#pragma pack(pop)

int main()
{
	wchar_t pakFile[] = L"E:/Games/MenofWarAssaultSquad2Origins/resource/entity/c2.pak";
	if (!PathFileExistsW(pakFile)) {
		std::wcerr << "The file \"" << pakFile << "\" not exists." << std::endl;
		return 1;
	}
	std::wstring resultFile(PathFindFileNameW(pakFile));
	resultFile += L".csv";

	//using  CsvLine = vector<string>;
	//using ParseResult = vector<CsvLine>;

	//ParseResult pr;

	//int g_columns = 10;

	//if (PathFileExistsW(resultFile.c_str())) {
	//	std::ifstream fs(resultFile);
	//	text::csv::csv_istream csvInput(fs);
	//}

	ofstream ofs(resultFile);
	csv_ostream ocsv(ofs);

	// header
	ocsv << "Name"	// 1
		<< "Offset"	// 2
		<< "Min"	// 3
		<< "Max"	// 4
		<< "SkinsCount"	// 5
		<< "SkinsName"		// 6
		<< "Unknown1"		// 7
		<< "Hex1"		// 7
		<< "Dec1"		// 7
		<< "TriangleStart"		// 8
		<< "FacesTriangles"	// 9
		<< "MaterialType"	// 10
		<< "BumpColor"		// 11
		<< "Material"		// 12
		<< "Skins2"		// 13
		<< "SkinsIndex"	// 14
		<< "VerticesCount"	// 15
		<< "VertexStride"	// 16
		<< "Unknown5"		// 17
		<< "Hex5"		// 17
		<< "Dec5"		// 17
		<< "VertexBytes"	// 18
		<< "IndicesCount"	// 19
		<< "IndexBytes"	// 20
		<< "Adja"	// 21
		<< "Shdw"	// 22
		<< csv::endl;

	SafeHandle hFile = CreateFileW(pakFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		OutputDebugStringW(L"CreateFile failed.\n");
		wcerr << L"CreateFile failed.\n" << std::endl;
		return -1;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		OutputDebugStringW(L"GetFileSizeEx failed.\n");
		wcerr << L"GetFileSizeEx failed." << std::endl;
		return -2;
	}

	struct _FileBlock {
		_ZipLocalFileHeader header;
		DWORD offset;
	};

	DWORD signature;
	DWORD readedBytes;
	while (ReadFile(hFile, &signature, 4, &readedBytes, NULL)) {
		switch (signature) {
		case LocalFileHeader:
		{
			auto fileBlock = std::make_shared<_FileBlock>();
			DWORD stToRead = sizeof(_ZipLocalFileHeader);
			if (!ReadFile(hFile, &fileBlock->header, stToRead, &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for Local file header failed.\n");
				break;
			}
			//char* fileName = new char[fileBlock->header.fname_len + 1]{ 0 };
			auto fileName = std::make_unique<char[]>(fileBlock->header.fname_len + 1);
			ReadFile(hFile, fileName.get(), fileBlock->header.fname_len, &readedBytes, NULL);

			if (fileBlock->header.extra_field_len) {
				SetFilePointer(hFile, fileBlock->header.extra_field_len, 0, FILE_CURRENT);
			}
			fileBlock->offset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
			if (fileBlock->header.comp_size) {
				if (strncmp(".ply", PathFindExtensionA(fileName.get()), 4)) {
					SetFilePointer(hFile, fileBlock->header.comp_size, 0, FILE_CURRENT);
					break;
				}
				OutputDebugStringA("    \t\t----***** SPLIT *****----\n");

				ocsv << fileName.get();	// 1,"Name"
				OutputDebugStringA("\t01. Name: ");
				OutputDebugStringA(fileName.get());
				OutputDebugStringA("\n");

				char buf[256] = { 0 };
				int nPos = 0;
				sprintf_s(buf, "0X%08X", fileBlock->offset);
				ocsv << buf;	// 2,"Offset"
				OutputDebugStringA("\t02. Offset: ");
				OutputDebugStringA(buf);
				OutputDebugStringA("\n");

				auto dataPtr = std::make_unique<byte[]>(fileBlock->header.comp_size);
				ReadFile(hFile, dataPtr.get(), fileBlock->header.comp_size, &readedBytes, NULL);
				const byte* meshData = dataPtr.get();

				// File Header
				if (*(PUINT64)meshData != *(PUINT64)"EPLYBNDS") {
					OutputDebugStringW(L"Invalid ply file.\n");
					cout << "\"" << fileName.get() << "\" is invalid ply file." << std::endl;
					SetFilePointer(hFile, fileBlock->header.comp_size, 0, FILE_CURRENT);
					break;
				}
				meshData += 8;

				float x = *(float*)meshData;
				meshData += 4;
				float y = *(float*)meshData;
				meshData += 4;
				float z = *(float*)meshData;
				meshData += 4;
				sprintf_s(buf, "%f,%f,%f", x, y, z);
				ocsv << buf;	// 3,"Min"
				OutputDebugStringA("\t03. Min: ");
				OutputDebugStringA(buf);
				OutputDebugStringA("\n");

				x = *(float*)meshData;
				meshData += 4;
				y = *(float*)meshData;
				meshData += 4;
				z = *(float*)meshData;
				meshData += 4;
				sprintf_s(buf, "%f,%f,%f", x, y, z);
				ocsv << buf;	// 4,"Max"
				OutputDebugStringA("\t04. Max: ");
				OutputDebugStringA(buf);
				OutputDebugStringA("\n");

				//std::vector<UINT16> m_SkinIndices;

				UINT32 numVertices = 0;
				UINT16 vertexStride = 0;
				UINT32 numIndices = 0;


				const UINT32 SKIN = 0x4e494b53; // "SKIN";
				const UINT32 MESH = 0x4853454d; // "MESH";
				const UINT32 VERT = 0x54524556; // "VERT";
				const UINT32 INDX = 0x58444e49; // "INDX";
				const UINT32 ADJA = 0x414a4441; // "ADJA";
				const UINT32 SHDW = 0x57444853; // "SHDW";
				int skinBlock = 0;
				int meshBlock = 0;
				UINT32 skinsCount = 0;
				while ((size_t)(meshData - dataPtr.get()) < readedBytes - 4) {
					UINT32 u1, u2;
					UINT32 magicK = *(PUINT32)meshData;
					meshData += 4;
					switch (magicK) {
					case SKIN:
					{
						OutputDebugStringA("Skin*******************************\n");
						skinBlock++;
						if (skinBlock > 1) {
							sprintf_s(buf, "Skin blocks %i\n", skinBlock);
							OutputDebugStringA(buf);
							ocsv << csv::endl << fileName.get() << "" << "" << "";
						}
						//skins, = struct.unpack("<I", f.read(4))
						//	print("Number of skins: %i at %s" % (skins, hex(f.tell())))
						//	for i in range(0, skins) :
						//		skin_name_length, = struct.unpack("B", f.read(1))
						//		print("Skin name length:", hex(skin_name_length))
						//		skin_name = f.read(skin_name_length)
						//		print("Skin name:", skin_name)
						skinsCount = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)skinsCount;	// 5,"Skins Count"
						sprintf_s(buf, "%i", skinsCount);
						OutputDebugStringA("\t05. Skins Count: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						nPos = 0;
						for (UINT32 s = 0; s < skinsCount; s++) {
							std::string skin((LPCSTR)meshData + 1, meshData[0]);
							meshData += 1 + meshData[0];
							nPos += sprintf_s(buf + nPos, 255 - nPos, "%s,", skin.c_str());
						}
						if (nPos)
							buf[nPos - 1] = 0;
						ocsv << buf;	// 6, "Skins Name"
						OutputDebugStringA("\t06. Skins Name: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");
					}
					break;
					case MESH:
					{
						OutputDebugStringA("Mesh*******************************\n");
						meshBlock++;
						if (meshBlock > 1) {
							sprintf_s(buf, "Mesh blocks %i\n", meshBlock);
							OutputDebugStringA(buf);
							ocsv << csv::endl << fileName.get() << "" << "" << "" << "" << "";
						}
						else if (!skinBlock) {
							ocsv << "" << "";
						}
						//# read 
						//	f.read(0x8)
						//	triangles, = struct.unpack("<I", f.read(4))
						//	print("Number of triangles:", triangles)
						//	self.material_info, = struct.unpack("<I", f.read(4))
						//	print("Material info:", hex(self.material_info))
						//	if self.material_info in SUPPORTED_FORMAT :
						//if self.material_info == 0x0404 :
						//	pass
						//	elif self.material_info == 0x0C14 :
						//	pass
						//else :
						//	vert = f.read(0x4)
						//	else:
						//raise Exception("Unsupported material type")
						//	material_name_length, = struct.unpack("B", f.read(1))
						//	print("Material name length:", hex(material_name_length))
						//	material_file = f.read(material_name_length)
						//	print("Material file:", material_file)
						//	# read some more unknown data
						//	if self.material_info == 0x0C14:
						//f.read(3)
						u1 = *(PUINT32)meshData;
						OutputDebugStringA("\t07. Unknown1: ");
						sprintf_s(buf, "0X%08X", (UINT32)(meshData - dataPtr.get()));
						meshData += 4;	// some unknown data
						ocsv << buf;	// 7, "Unknown1"
						OutputDebugStringA(buf);
						OutputDebugStringA(", ");
						sprintf_s(buf, "0x%08x", u1);
						ocsv << buf;	// 7, "Hex1"
						OutputDebugStringA(buf);
						OutputDebugStringA(", ");
						sprintf_s(buf, "%i", u1);
						ocsv << buf;	// 7, "Dec1"
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						u1 = *(PUINT32)meshData;
						sprintf_s(buf, "%i", u1);
						meshData += 4;	// some unknown data
						ocsv << buf;	// 8, "Triangle Start"
						OutputDebugStringA("\t08. Triangle Start: ");
						OutputDebugStringA(buf);
						OutputDebugStringA(" (x 3 == index offset)\n");

						u1 = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)u1;	// 9, "Faces or Triangles"
						sprintf_s(buf, "%i", u1);
						OutputDebugStringA("\t09. Faces Triangles: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						u1 = *(PUINT32)meshData;
						meshData += 4;
						sprintf_s(buf, "0X%08X", u1);
						ocsv << buf;	// 10, "Material Type"
						OutputDebugStringA("\t10. Material Type: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						// 0x0644, 0x0604, 0x0404, 0x0704, 0x0744, 0x0C14
						//const UINT32 MAT_404 = 0x0404;
						//const UINT32 MAT_405 = 0x0405;
						//const UINT32 MAT_444 = 0x0444;
						//const UINT32 MAT_445 = 0x0445;
						//const UINT32 MAT_504 = 0x0504;
						//const UINT32 MAT_505 = 0x0505;
						//const UINT32 MAT_544 = 0x0544;
						//const UINT32 MAT_604 = 0x0604;
						//const UINT32 MAT_605 = 0x0605;
						//const UINT32 MAT_644 = 0x0644;
						//const UINT32 MAT_704 = 0x0704;
						//const UINT32 MAT_744 = 0x0744;
						//const UINT32 MAT_C14 = 0x0C14;
						//const UINT32 MAT_C54 = 0x0C54;
						//const UINT32 MAT_F14 = 0x0F14;
						switch (u1) {
						case 0x0005:
						case 0x0401:
						case 0x0404:	// simple
						case 0x0405:	// simple
						case 0x0406:
						case 0x040c:
						case 0x0444:
						case 0x0445:
						case 0x0504:
						case 0x0505:
						case 0x0506:
						case 0x0544:
						case 0x0c14:
						case 0x0c15:
						case 0x0c54:
						case 0x0c55:
						case 0x0d14:
							ocsv << "";
							OutputDebugStringA("\t11. Simple material.\n");
							break;
						case 0x0604:	// bump
						case 0x0605:
						case 0x0644:	// bump
						case 0x0645:
						case 0x0704:
						case 0x0705:
						case 0x0744:	// bump
						case 0x0745:
						case 0x0e14:
						case 0x0f14:
						case 0x0f15:
						case 0x0f54:
						case 0x0f55:
							u2 = *(PUINT32)meshData;	// bump color
							sprintf_s(buf, "(%u,%u,%u,%u)", meshData[0], meshData[1], meshData[2], meshData[3]);
							meshData += 4;
							ocsv << buf;	// 11, "Bump Color"
							OutputDebugStringA("\t11. Bump material color: ");
							OutputDebugStringA(buf);
							OutputDebugStringA("\n");
							break;
							//case 0x0e15:
							//case 0x0e54:
							//case 0x0e55:
						default:
							u2 = *(PUINT32)meshData;
							sprintf_s(buf, "0X%08X: 0x%04x, %i, %f", (UINT32)(meshData - dataPtr.get()), u2, u2, *(float*)meshData);
							meshData += 4;
							ocsv << buf;	// 11, "Bump Color"
							OutputDebugStringA("\t11. Bump material color default: ");
							OutputDebugStringA(buf);
							OutputDebugStringA("\n");
							break;
						}
						std::string material((LPCSTR)meshData + 1, meshData[0]);
						meshData += 1 + meshData[0];
						ocsv << material;	// 12, "Materials"
						OutputDebugStringA("\t12. Materials: ");
						OutputDebugStringA(material.c_str());
						OutputDebugStringA("\n");

						//if (MAT_C14 == matType) {
						//	nPos = sprintf(buf, "0X%08X: %x,%x,%x ",
						//		meshData * dataPtr.get(), meshData[0], meshData[1], meshData[2]);
						//	nPos = sprintf_s(buf + nPos, 255 - nPos, "%c,%c,%c ",
						//		meshData[0], meshData[1], meshData[2]);
						//	nPos = sprintf_s(buf + nPos, 255 - nPos, "%i,%i,%i",
						//		meshData[0], meshData[1], meshData[2]);
						//	meshData += 3;
						//	ocsv << buf;	// 13, "Unknown 4(x,c,i)"
						//}
						if (skinBlock) {
							UINT16 skinsA2 = *(PUINT16)meshData;
							meshData += 2;
							ocsv << skinsA2;	// 13, "Skins2"
							sprintf_s(buf, "%i", skinsA2);
							OutputDebugStringA("\t13. Skins2: ");
							OutputDebugStringA(buf);
							OutputDebugStringA("\n");
							//assert(skinsCount + 1 == skinsA2);

							nPos = 0;
							for (UINT16 p = 0; p < skinsA2 - 1; p++) {
								nPos += sprintf_s(buf + nPos, 255 - nPos, "%i,", meshData[0]);
								meshData++;
								//if (p == skinsA2)
								//	break;
							}
							if (nPos)
								buf[nPos - 1] = 0;
							ocsv << buf;	// 14, "Skins Index"
							OutputDebugStringA("\t14. Skins Index: ");
							OutputDebugStringA(buf);
							OutputDebugStringA("\n");
						}
						else {
							ocsv << "" << "";
							OutputDebugStringA("\t05/06. nothing.\n");
							OutputDebugStringA("\t13/14. nothing.\n");
						}
					}
					break;
					case VERT:
					{
						OutputDebugStringA("Vert*******************************\n");
						//verts, = struct.unpack("<I", f.read(4))
						//                 print("Number of verts: %i at %s" % (verts, hex(f.tell())))
						//                 vertex_description, = struct.unpack("<I", f.read(4))
						//                 print("Vertex description:", hex(vertex_description))
						//                 for i in range(0, verts):
						//                     if vertex_description == 0x00010024:
						//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffff4xff", f.read(36))
						//                     elif vertex_description == 0x00070020:
						//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff", f.read(32))
						//                     elif vertex_description == 0x00070028:
						//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff8x", f.read(40))
						//                     elif vertex_description == 0x00070030:
						//                         vx,vy,vz,nx,ny,nz,U,V = struct.unpack("ffffffff16x", f.read(48))
						//                     else:
						//                         raise Exception("Unknown format: %s" % hex(vertex_description))
						//                     if verbose:
						//                         print("Vertex %i: " % i,vx,vy,vz)
						//                     self.positions.append((vx,vy,vz))
						//                     self.normals.append((nx,ny,nz))
						//                     if not self.translate_uv_y:
						//                         self.UVs.append((U,V))
						//                     else:
						//                         self.UVs.append((U,V+1.0))
						//                 print("Vertex info ends at:",hex(f.tell()))
						numVertices = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)numVertices;	// 15, "Vertices Count"
						sprintf_s(buf, "%i", numVertices);
						OutputDebugStringA("\t15. Vertices Count: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						vertexStride = *(PUINT16)meshData;
						meshData += 2;
						ocsv << (long)vertexStride;	// 16, "Vertex Stride"
						sprintf_s(buf, "%i", vertexStride);
						OutputDebugStringA("\t16. Vertex Stride: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						u1 = *(PUINT16)meshData;
						sprintf_s(buf, "0X%08X", (UINT32)(meshData - dataPtr.get()));
						meshData += 2;	// some unknown data
						ocsv << buf;	// 17, "Unknown5"
						OutputDebugStringA(buf);
						OutputDebugStringA(", ");
						sprintf_s(buf, "0x%02x", u1);
						ocsv << buf;	// 17, "Hex5"
						OutputDebugStringA(buf);
						OutputDebugStringA(", ");
						sprintf_s(buf, "%i", u1);
						ocsv << buf;	// 17, "Dec5"
						OutputDebugStringA("\t17. Unknown5: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");
						if (7 != u1)
						{
							OutputDebugStringA("check here.\n");

						}
						ULONG verticesBytes = numVertices * vertexStride;
						sprintf_s(buf, "0X%08X: 0x%08x, %i", (UINT32)(meshData - dataPtr.get()), verticesBytes, verticesBytes);
						ocsv << buf;	// 18, "Vertex Bytes"
						OutputDebugStringA("\t18. Vertex Bytes: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						meshData += verticesBytes;
					}
					break;
					case INDX:
					{
						OutputDebugStringA("Indx*******************************\n");
						//idx_count, = struct.unpack("<I", f.read(4))
						//	print("Indeces:", idx_count)
						//	for i in range(0, idx_count / 3) :
						//		i0, i1, i2 = struct.unpack("<HHH", f.read(6))
						//		if verbose :
						//			print("Face %i:" % i, i0, i1, i2)
						//			if self.material_info == 0x0744 :
						//				self.indeces.append((i2, i1, i0))
						//			else :
						//				self.indeces.append((i0, i1, i2))
						//				print("Indces end at", hex(f.tell() * 1))
						numIndices = *(PUINT32)meshData;
						meshData += 4;
						ocsv << (long)numIndices;	// 19, "Indices Count"
						sprintf_s(buf, "%i", numIndices);
						OutputDebugStringA("\t19. Indices Count: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						ULONG indicesBytes = numIndices * sizeof(WORD);
						sprintf_s(buf, "0X%08X: 0x%08x, %i", (UINT32)(meshData - dataPtr.get()), indicesBytes, indicesBytes);
						ocsv << buf;	// 20, "Index Bytes"
						OutputDebugStringA("\t20. Index Bytes: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						meshData += indicesBytes;
					}
					break;
					case ADJA:
					{
						OutputDebugStringA("Adja*******************************\n");
						u1 = *(PUINT32)meshData;
						meshData += 4;
						sprintf_s(buf, "0X%08X: %i", (UINT32)(meshData - dataPtr.get()), u1);
						ocsv << buf;	// 21, "Adja"
						OutputDebugStringA("\t21. Adja: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						meshData += u1 * 6;
					}
					break;
					case SHDW:
					{
						OutputDebugStringA("Shdw*******************************\n");
						u1 = *(PUINT32)meshData;
						meshData += 4;
						sprintf_s(buf, "0X%08X: %i", (UINT32)(meshData - dataPtr.get()), u1);
						ocsv << buf;	// 22, "Shdw"
						OutputDebugStringA("\t22. Shdw: ");
						OutputDebugStringA(buf);
						OutputDebugStringA("\n");

						meshData += u1;
					}
					break;
					default:
						sprintf_s(buf, "Invalid ply block in 0X%08X\n", (UINT32)(meshData - dataPtr.get() - 4));
						OutputDebugStringA(buf);
					}
				}
				ocsv << csv::endl;
				OutputDebugStringA("\n");
			}
		}
		break;
		case DataDescriptor:
			OutputDebugStringW(L"Data descriptor not supported.\n");
			break;
		case CentralDirectoryFileHeader:
		{
			_ZipCDFHeader cdfHeader;
			if (!ReadFile(hFile, &cdfHeader, sizeof(cdfHeader), &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for Central directory file header failed.\n");
				break;
			}
			char* fileName = new char[cdfHeader.fname_len + 1]{ 0 };
			ReadFile(hFile, fileName, cdfHeader.fname_len, &readedBytes, NULL);
			delete[] fileName; fileName = nullptr;

			if (cdfHeader.extra_field_len) {
				SetFilePointer(hFile, cdfHeader.extra_field_len, 0, FILE_CURRENT);
			}

			if (cdfHeader.fcomment_len) {
				SetFilePointer(hFile, cdfHeader.fcomment_len, 0, FILE_CURRENT);
			}
		}
		break;
		case EOCD:
		{
			_ZipEOCD eocdBlock;
			DWORD stToRead = sizeof(eocdBlock);
			if (!ReadFile(hFile, &eocdBlock, stToRead, &readedBytes, NULL)) {
				OutputDebugStringW(L"ReadFile for End of central directory record failed.\n");
				break;
			}

			if (eocdBlock.lenComment) {
				SetFilePointer(hFile, eocdBlock.lenComment, 0, FILE_CURRENT);
			}
		}
		break;
		default:
			OutputDebugStringW(L"Unknown zip block.\n");
			break;
		}
		DWORD pos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
		if (pos >= fileSize.LowPart) {
			OutputDebugStringW(L"File Ended.\n");
			break;;
		}
	}

	return 0;
}

