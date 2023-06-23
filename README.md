<h1 align="center">lazy payload poc</h1>
<p align="center">
  <img src="https://img.shields.io/badge/Windows--x86__64-supported-44CC11?style=flat-square"/>
  <img src="https://img.shields.io/badge/Windows--x86-supported-44CC11?style=flat-square"/>
  <a href="https://mit-license.org/"/>
    <img src="https://img.shields.io/github/license/0xvpr/lazy-payload-poc?style=flat-square&color=44CC11"/>
  </a>
  <img src="https://img.shields.io/github/actions/workflow/status/0xvpr/lazy-payload-poc/docker_build.yml?style=flat-square"/>
  <img src="https://img.shields.io/github/actions/workflow/status/0xvpr/lazy-payload-poc/windows_runtime_test.yml?label=tests"/>
</p>
<br>

# Overview
This is a proof of concept for leveraging the behavior of kernel32 and ntdll kernel loading  
in order to write small, executable, shellcode that will run in an arbitrary process.

# Demo
[ TODO ]

# Building Using Docker (Recommended)
```bash
git clone https://github.com/0xvpr/lazy-payload-poc.git lazy-payload-poc && cd lazy-payload-poc
make docker-container
make docker-build
```
