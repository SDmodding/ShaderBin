#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

#define THEORY_IMPL
#define THEORY_DUCKTAPE
#define THEORY_SLIM_BUILD
#include "theory/theory.hh"

using namespace UFG;

#include "core.hh"

qResourceInventory gShaderBinaryInventory = { "iShaderBinaryInventory", RTypeUID_ShaderBinary, ChunkUID_ShaderBinary };

void LoadShader(const qString& path, bool rebuild)
{
	auto* bin = core::NewShaderBinary(path);

	if (auto* bin_dup = gShaderBinaryInventory.Get(bin->mNode.mUID))
	{
		if (!rebuild)
		{
			qPrintf("[ WARN ] '%s' already exists. Use '-rebuild' to update it.\n", bin->mShaderName);
			return;
		}

		gShaderBinaryInventory.Remove(bin_dup);
	}

	gShaderBinaryInventory.Add(bin);
}

void LoadShaderFiles(const qString& find_path, bool rebuild)
{
	WIN32_FIND_DATAA wFindData = { 0 };
	HANDLE hFind = FindFirstFileA(find_path, &wFindData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	qString folder = find_path.GetFilePath() + "\\";

	do
	{
		const qString file_path = (folder + wFindData.cFileName);

		if (wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		if (file_path.ToLower().EndsWith(".hlsl")) {
			LoadShader(file_path, rebuild);
		}
	} while (FindNextFileA(hFind, &wFindData));

	FindClose(hFind);
}

int main(int argc, char** argv)
{
	UFG::qInit(0);
	Illusion::gEngine.Init();

	if (1 >= argc)
	{
		qPrintf("ERROR: Missing input file!\n");
		return 1;
	}

	qString output_path = "shaders";

	bool unpack = 0;
	bool rebuild = 0;

	for (int i = 1; argc > i; ++i)
	{
		const qString arg = argv[i];
		const qString path = arg.ToLower();

		if (path.EndsWith(".bin"))
		{
			qString temp_path = arg;
			temp_path.ReplaceString(".perm.bin", ".temp.bin", 1);

			StreamResourceLoader::LoadResourceFile(temp_path);
			continue;
		}

		if (path.EndsWith(".hlsl"))
		{
			if (unpack) {
				continue;
			}

			LoadShader(arg, rebuild);
			continue;
		}

		if (arg.EndsWith("\\*") || arg.EndsWith("/*"))
		{
			if (unpack) {
				continue;
			}

			LoadShaderFiles(arg, rebuild);
			continue;
		}

		if (auto* param = core::GetParamValue(arg, "-output="))
		{
			output_path = param;
			continue;
		}

		if (core::GetParamValue(arg, "-unpack")) { unpack = 1; continue; }
		if (core::GetParamValue(arg, "-rebuild")) { rebuild = 1; continue; }
	}

	if (unpack)
	{
		output_path = output_path.GetFilename();
		qCreateDirectory(output_path);

		for (auto* resource : gShaderBinaryInventory.mResourceDatas)
		{
			auto* bin = static_cast<Illusion::ShaderBinary*>(resource);
			auto* data = bin->mData.Get();
			if (!data) {
				continue;
			}

			qString shader_path = { "%s/%s", output_path.mData, bin->mShaderName };
			auto* file = qOpen(shader_path, QACCESS_WRITE);
			if (!file) {
				continue;
			}

			qWrite(file, data, bin->mDataByteSize);
			qClose(file);
		}
	}


	UFG::qChunkFileBuilder chunk_builder;

	//chunk_builder.CreateBuilder("PC64", output_path + ".perm.bin");
	//chunk_builder.CloseBuilder(0, true);

	chunk_builder.CreateBuilder("PC64", output_path + ".temp.bin");

	for (auto* resource : gShaderBinaryInventory.mResourceDatas)
	{
		auto* bin = static_cast<Illusion::ShaderBinary*>(resource);

		qMemSet(&bin->mNode.mParent, 0, sizeof(bin->mNode.mParent));
		qMemSet(&bin->mNode.mChild, 0, sizeof(bin->mNode.mChild));
		qMemSet(&bin->mResourceHandles, 0, sizeof(bin->mResourceHandles));

		chunk_builder.BeginChunk(ChunkUID_ShaderBinary, "Illusion.ShaderBinary");

		chunk_builder.Write(bin, sizeof(Illusion::ShaderBinary) + bin->mDataByteSize);

		chunk_builder.EndChunk(ChunkUID_ShaderBinary);
	}

	chunk_builder.CloseBuilder(0, true);

	_exit(0);
}