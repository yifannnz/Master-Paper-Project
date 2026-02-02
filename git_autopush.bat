@echo off
setlocal
pushd %~dp0

REM Ensure we're inside a git repository
for /f "delims=" %%i in ('git rev-parse --is-inside-work-tree 2^>nul') do set INSIDE=%%i
if /i not "%INSIDE%"=="true" (
  echo Not inside a git repository.
  popd
  exit /b 1
)

git add -A

git diff --cached --quiet
if %errorlevel%==0 (
  echo No staged changes to commit.
) else (
  git commit -m "temp commit"
)

git push

popd
endlocal
