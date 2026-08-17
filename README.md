# ShaderBin
A simple CLI tool for working with pre-existing `.temp.bin` shader files.

It can:
* Rebuild an existing `.temp.bin` using new HLSL shader files.
* Create a new `.temp.bin` from HLSL shaders.
* Unpack an existing `.temp.bin` into individual shader blob files.
* Replace existing shaders during a rebuild.

## Basic Usage

### Rebuild a shader binary

Rebuild `shaders.temp.bin` using HLSL files from the `shaders` directory:

```cmd
ShaderBinPC64.exe -rebuild "shaders.temp.bin" "shaders/*" -output="new_shaders"
```

This loads the existing shader binary, matches the provided HLSL shaders, and writes the rebuilt binary to the specified output path.

### Unpack a shader binary

Export the resources contained in a `.temp.bin` file:

```cmd
ShaderBinPC64.exe -unpack "shaders.temp.bin"
```

This creates individual shader blob files from the resources stored in the binary.

## Wildcards

ShaderBin supports a single `*` wildcard when specifying HLSL files.

A wildcard matches files **only in the specified directory** and does not search subdirectories.

For example:

```cmd
ShaderBinPC64.exe -rebuild "shaders.temp.bin" "shaders/*" -output="new_shaders"
```

The `shaders/*` pattern loads files directly inside the `hlsl` directory.

Subdirectories are not searched recursively.

## Command-Line Options

<table>
  <thead>
    <tr>
      <th>Option</th>
      <th>Description</th>
      <th>Example</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>-output=&lt;path&gt;</code></td>
      <td>Optional output path. The specified path is used as the output file name without an extension.</td>
      <td><code>-output=shaders</code></td>
    </tr>
    <tr>
      <td><code>-rebuild</code></td>
      <td>Enables rebuilding an existing shader binary and allows new shaders to replace pre-existing ones.</td>
      <td><code>-rebuild</code></td>
    </tr>
    <tr>
      <td><code>-unpack</code></td>
      <td>Exports each resource from a <code>.temp.bin</code> file as an individual shader blob file.</td>
      <td><code>-unpack</code></td>
    </tr>
  </tbody>
</table>