SHELL := /bin/bash
.DEFAULT_GOAL := help
# Preparing replaces the generated tree: serialize compile/test/upload.
.NOTPARALLEL:

AUTO_PORTS = $(shell arduino-cli board list 2>/dev/null | awk '$$1 ~ "^/dev/cu\\.usbmodem" { print $$1 }')
PORT ?= $(if $(filter 1,$(words $(AUTO_PORTS))),$(firstword $(AUTO_PORTS)),)
VERBOSE ?= 0
SPEAKER ?= passive
DISPLAY ?= on
WIFI ?= on
# Leave empty until the real ALT gearing has been measured.
ALT_STEPS_PER_DEGREE ?=
PROFILE_ARGS = --speaker "$(SPEAKER)" --display "$(DISPLAY)" --wifi "$(WIFI)" $(if $(strip $(ALT_STEPS_PER_DEGREE)),--alt-steps-per-degree "$(ALT_STEPS_PER_DEGREE)")

.PHONY: help setup list detect prepare compile test check install upload all clean quality previews

help:
	@printf '%s\n' 'OnStep200P' '' \
	  'make setup       Instala a toolchain fixada pelo projeto' \
	  'make prepare     Prepara a árvore do OnStepX' \
	  'make compile     Prepara e compila o firmware' \
	  'make test        Prepara a fonte e roda os testes host' \
	  'make check       Testa e compila (sem gravar)' \
	  'make quality     Audita arquivos públicos, links e previews' \
	  'make previews    Regenera previews OLED com dados fictícios' \
	  'make list        Lista portas; make detect seleciona uma porta única' \
	  'make install     Grava o último build válido (alias: upload)' \
	  'make all         Testa, compila e grava, nessa ordem' \
	  'make clean       Remove somente as árvores geradas' '' \
	  'Opções: VERBOSE=1 PORT=/dev/cu.usbmodemXXXX' \
	  'SPEAKER=off|active|passive (padrão: passive)' \
	  'DISPLAY=on|off WIFI=on|off (padrão: on)' \
	  'ALT_STEPS_PER_DEGREE=<valor medido> (vazio = provisório)' '' \
	  'Exemplo: make check SPEAKER=passive VERBOSE=1' \
	  'Upload usa o perfil já compilado; opções exigem recompilar.'

setup:
	@./scripts/setup-toolchain.sh

quality:
	@python3 scripts/audit-public.py
	@python3 scripts/check-docs.py
	@python3 scripts/render-previews.py --check

previews:
	@python3 scripts/render-previews.py

list:
	@arduino-cli board list

detect:
	@if [[ -n "$(PORT)" ]]; then \
	  [[ -c "$(PORT)" ]] || { echo "Erro: porta serial inexistente: $(PORT)" >&2; exit 1; }; \
	  echo "Porta: $(PORT)"; \
	elif [[ "$(words $(AUTO_PORTS))" -eq 0 ]]; then \
	  echo "Erro: nenhuma porta /dev/cu.usbmodem* encontrada. Use make list ou PORT=..." >&2; exit 1; \
	else \
	  echo "Erro: múltiplas portas USB: $(AUTO_PORTS). Use PORT=..." >&2; exit 1; \
	fi

prepare:
	@./scripts/prepare-source.sh $(PROFILE_ARGS)

# compile.sh already prepares the source once.
compile:
	@VERBOSE="$(VERBOSE)" ./scripts/compile.sh $(PROFILE_ARGS)

test: prepare
	@bash ./scripts/test.sh

check: test compile

install: detect
	@VERBOSE="$(VERBOSE)" ./scripts/upload.sh "$(PORT)"

upload: install

all: check install

clean:
	@rm -rf -- "$(CURDIR)/build/onstepx" "$(CURDIR)/.build/OnStepX"
