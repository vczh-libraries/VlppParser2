param()

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..")

# Remove and recreate folders
foreach ($folder in @("Xml", "Json", "Workflow")) {
    $folderPath = Join-Path $scriptDir $folder
    if (Test-Path $folderPath) {
        Remove-Item $folderPath -Recurse -Force
    }
    New-Item $folderPath -ItemType Directory | Out-Null
}

# Copy .d.ts files
Copy-Item (Join-Path $repoRoot "Source\Xml\Generated\XmlAst_Json.d.ts") (Join-Path $scriptDir "Xml\XmlAst_Json.d.ts")
Copy-Item (Join-Path $repoRoot "Source\Json\Generated\JsonAst_Json.d.ts") (Join-Path $scriptDir "Json\JsonAst_Json.d.ts")
Copy-Item (Join-Path $repoRoot "Test\Source\BuiltIn-Workflow\Generated\WorkflowAst_Json.d.ts") (Join-Path $scriptDir "Workflow\WorkflowAst_Json.d.ts")

# Generate .ts files from JSON test outputs
function GenerateTsFiles($jsonDir, $tsDir, $dtsModule, $importNames, $typeAnnotation) {
    $jsonFiles = Get-ChildItem (Join-Path $repoRoot $jsonDir) -Filter "*.json"
    foreach ($jsonFile in $jsonFiles) {
        $baseName = $jsonFile.BaseName -replace '[^\w]', '_'
        $jsonContent = Get-Content -LiteralPath $jsonFile.FullName -Raw
        $tsContent = "import type { $importNames } from './$dtsModule';`n`nconst json: $typeAnnotation = $jsonContent;`n"
        $tsPath = Join-Path $tsDir "$baseName.ts"
        Set-Content -Path $tsPath -Value $tsContent -Encoding UTF8
    }
}

function GenerateWorkflowTsFiles($jsonDir, $tsDir, $dtsModule) {
    $jsonFiles = Get-ChildItem (Join-Path $repoRoot $jsonDir) -Filter "*.json"
    foreach ($jsonFile in $jsonFiles) {
        $baseName = $jsonFile.BaseName -replace '[^\w]', '_'
        $jsonContent = Get-Content -LiteralPath $jsonFile.FullName -Raw
        $jsonObj = $jsonContent | ConvertFrom-Json
        $astType = $jsonObj.'$ast'
        $tsContent = "import type { $astType } from './$dtsModule';`n`nconst json: $astType = $jsonContent;`n"
        $tsPath = Join-Path $tsDir "$baseName.ts"
        Set-Content -Path $tsPath -Value $tsContent -Encoding UTF8
    }
}

GenerateTsFiles "Test\ParserLog\BuiltIn-Xml" (Join-Path $scriptDir "Xml") "XmlAst_Json" "Element, Document" "Element | Document"
GenerateTsFiles "Test\ParserLog\BuiltIn-Json" (Join-Path $scriptDir "Json") "JsonAst_Json" "Object as JsonObject, Array as JsonArray" "JsonObject | JsonArray"
GenerateWorkflowTsFiles "Test\ParserLog\BuiltIn-Workflow" (Join-Path $scriptDir "Workflow") "WorkflowAst_Json"

Write-Host "Prepared TypeScript test files successfully."
