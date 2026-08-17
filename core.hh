#pragma once

namespace core
{
	const char gShaderStagePrefixes[Illusion::Shader::MAX_STAGES] = { 'v', 'p', 'c', 'g', 'h', 'd' };

	Illusion::Shader::StageType DetermineShaderStage(const char* file_name, const char* buf)
	{
		for (int i = 0; Illusion::Shader::MAX_STAGES > i; ++i)
		{
			const qString suffix = { ".%cSBIN", qToUpper(gShaderStagePrefixes[i]) };
			if (!qStringFindInsensitive(file_name, suffix)) {
				continue;
			}

			return static_cast<Illusion::Shader::StageType>(i);
		}

		return Illusion::Shader::STAGE_INVALID;
	}

	Illusion::ShaderBinary* NewShaderBinary(const char* name, Illusion::Shader::StageType stage, const void* data, u32 data_size)
	{
		const u32 alloc_size = sizeof(Illusion::ShaderBinary) + data_size;
		auto* bin = static_cast<Illusion::ShaderBinary*>(qMalloc(alloc_size, "Illusion.ShaderBinary"));
		if (!bin) {
			return 0;
		}

		qMemSet(bin, 0, alloc_size);

		bin->mNode.mUID = qStringHashUpper32(name);
		bin->SetDebugName(name);
		bin->mTypeUID = RTypeUID_ShaderBinary;

		bin->mDataByteSize = data_size;
		bin->mShaderStageType = stage;

		if (data_size) 
		{
			void* bin_data = &bin[1];
			bin->mData.Set(bin_data);
			qMemCopy(bin_data, data, data_size);
		}

		qStringCopy(bin->mShaderName, sizeof(bin->mShaderName), name);

		return bin;
	}

	Illusion::ShaderBinary* NewShaderBinary(const qString& file_path)
	{
		const qString file_name = file_path.GetFilename();

		s64 shader_size = 0;
		char* shader = qReadEntireFile(file_path, &shader_size);
		if (!shader)
		{
			qPrintf("[ ERROR ] Failed to load file: '%s'\n", file_path.mData);
			return 0;
		}

		Illusion::Shader::StageType stage = DetermineShaderStage(file_name, shader);

		if (stage == Illusion::Shader::STAGE_INVALID)
		{
			qPrintf("[ ERROR ] Couldn't determine shader stage type for: '%s'\n", file_path.mData);
			qFree(shader);
			return 0;
		}

		const qString shader_target = { "%cs_5_0", gShaderStagePrefixes[stage] };

		ID3DBlob* blob = 0;
		ID3DBlob* errors = 0;

		HRESULT hr = D3DCompile(shader, static_cast<SIZE_T>(shader_size), 0, 0, 0, "main", shader_target, 0, 0, &blob, &errors);

		qFree(shader);

		if (FAILED(hr))
		{
			qPrintf("[ ERROR ] Failed to compile '%s' - ", file_path.mData);

			if (errors)
			{
				qPrintf("%s\n", static_cast<const char*>(errors->GetBufferPointer()));
				errors->Release();
			}
			else {
				qPrintf("?\n");
			}

			//qDebugBreak();
			return 0;
		}

		const qString shader_suffix = { ".%cSBIN", qToUpper(gShaderStagePrefixes[stage]) };
		const qString shader_name = file_path.GetFilenameWithoutExtension().GetFilenameWithoutExtension().ToUpper() + shader_suffix;

		auto* bin = NewShaderBinary(shader_name, stage, blob->GetBufferPointer(), static_cast<u32>(blob->GetBufferSize()));

		blob->Release();

		return bin;
	}

	const char* GetParamValue(const char* arg, const qString& param)
	{
		if (const char* find = qStringFindInsensitive(arg, param)) {
			return &find[param.Length()];
		}

		return 0;
	}

	const char* GetShaderEntryPoint(const char* buf, const char** profile)
	{
		static const char* sTable[][2] = {
			{ "VSMain", "vs_5_0" },
			{ "PSMain", "ps_5_0" },
			{ "GSMain", "gs_5_0" },
			{ "HSMain", "hs_5_0" },
			{ "DSMain", "ds_5_0" }
		};

		for (auto t : sTable)
		{
			if (qStringFindInsensitive(buf, t[0]))
			{
				if (profile) {
					*profile = t[1];
				}
				return t[0];
			}
		}

		return 0;
	}

	Illusion::Shader::StageType GetShaderStageType(const char* target)
	{
		switch (*target)
		{
		default: return Illusion::Shader::STAGE_INVALID;
		case 'v': return Illusion::Shader::STAGE_VERTEX;
		case 'p': return Illusion::Shader::STAGE_PIXEL;
		case 'g': return Illusion::Shader::STAGE_GEOMETRY;
		case 'h': return Illusion::Shader::STAGE_HULL;
		case 'd': return Illusion::Shader::STAGE_DOMAIN;
		}
	}
}