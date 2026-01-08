$files = Get-ChildItem -Path "Core" -Include *.c,*.h -Recurse

foreach ($file in $files) {
    $content = Get-Content -Path $file.FullName -Raw
    
    # Remove USER CODE BEGIN Header block
    $content = $content -replace '\/\* USER CODE BEGIN Header \*\/[\s\S]*?\/\* USER CODE END Header \*\/\s*', ''
    
    # Remove standalone comment blocks
    $content = $content -replace '\/\*-+\*\/\s*', ''
    
    # Remove USER CODE BEGIN/END pairs with empty content
    $content = $content -replace '\/\* USER CODE BEGIN \w+ \*\/\s*\/\* USER CODE END \w+ \*\/\s*', ''
    $content = $content -replace '\/\* USER CODE BEGIN \w+ \*\/\s*\n\s*\/\* USER CODE END \w+ \*\/\s*', ''
    
    # Save the file
    Set-Content -Path $file.FullName -Value $content -NoNewline
    Write-Host "Processed: $($file.FullName)"
}

Write-Host "Done!"
