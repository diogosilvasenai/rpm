-- ============================================================
--  Banco de dados: Velocímetro IoT com ESP32
--  Execute este script no phpMyAdmin ou via terminal MySQL
-- ============================================================

-- 1. Cria o banco de dados (caso não exista)
CREATE DATABASE IF NOT EXISTS velocimetro_iot
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

-- 2. Seleciona o banco criado
USE velocimetro_iot;

-- 3. Cria a tabela de leituras
CREATE TABLE IF NOT EXISTS leituras (
    id            INT UNSIGNED     NOT NULL AUTO_INCREMENT,
    velocidade    DECIMAL(6, 2)    NOT NULL COMMENT 'Velocidade em km/h',
    rpm           DECIMAL(8, 2)    NOT NULL COMMENT 'Rotações por minuto',
    registrado_em TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Data/hora do registro',
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================
--  Estrutura da tabela:
--   id            → chave primária autoincrementada
--   velocidade    → valor float com 2 casas decimais (ex: 32.75)
--   rpm           → valor float com 2 casas decimais (ex: 1450.00)
--   registrado_em → timestamp automático do momento do INSERT
-- ============================================================
