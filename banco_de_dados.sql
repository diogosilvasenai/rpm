CREATE DATABASE IF NOT EXISTS velocimetro_iot
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE velocimetro_iot;

CREATE TABLE IF NOT EXISTS leituras (
    id            INT UNSIGNED     NOT NULL AUTO_INCREMENT,
    velocidade    DECIMAL(6, 2)    NOT NULL COMMENT 'Velocidade em km/h',
    rpm           DECIMAL(8, 2)    NOT NULL COMMENT 'Rotações por minuto',
    registrado_em TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Data/hora do registro',
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
