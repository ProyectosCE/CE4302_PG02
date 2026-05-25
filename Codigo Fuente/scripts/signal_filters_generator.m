% Proyecto Grupal 2
% Arquitectura de Computadores II
%
% Generador de datasets para:
% Filtros FIR SIMD + GPU

clear;
clc;
close all;

rng(42);

% PATHS
current_script_path = fileparts(mfilename('fullpath'));

base_path = fullfile(current_script_path, '..');

datasets_base_path = fullfile(base_path, 'datasets');

if ~exist(datasets_base_path, 'dir')
    mkdir(datasets_base_path);
end

% CONFIGURACION GENERAL
NUM_FILTERS = 16;

SAMPLE_RATE = 48000;

% DATASETS
datasets = {
    struct( ...
        'name', 'small', ...
        'signal_size', 1e5, ...
        'filter_order', 128 ...
    ), ...

    struct( ...
        'name', 'medium', ...
        'signal_size', 1e6, ...
        'filter_order', 256 ...
    ), ...

    struct( ...
        'name', 'large', ...
        'signal_size', 1e7, ...
        'filter_order', 512 ...
    )
};

% GENERACION DE DATASETS
for d = 1:length(datasets)

    cfg = datasets{d};

    fprintf('\n=========================================\n');
    fprintf('Generando dataset: %s\n', cfg.name);
    fprintf('=========================================\n');

    dataset_path = fullfile(datasets_base_path, cfg.name);

    if ~exist(dataset_path, 'dir')
        mkdir(dataset_path);
    end

    N = cfg.signal_size;
    FILTER_ORDER = cfg.filter_order;

    fprintf('Cantidad de muestras: %d\n', N);
    fprintf('Orden FIR: %d\n', FILTER_ORDER);

    % GENERAR SEÑAL
    fprintf('Generando señal...\n');

    t = (0:N-1) / SAMPLE_RATE;

    % Frecuencias
    f1 = 200;
    f2 = 1200;
    f3 = 5000;
    f4 = 10000;

    % Señales
    signal_low  = 1.0 * sin(2*pi*f1*t);
    signal_mid  = 0.6 * sin(2*pi*f2*t);
    signal_high = 0.4 * sin(2*pi*f3*t);
    signal_vhf  = 0.3 * sin(2*pi*f4*t);

    % Ruido
    noise = 0.1 * randn(size(t));

    % Señal final
    signal = ...
        signal_low + ...
        signal_mid + ...
        signal_high + ...
        signal_vhf + ...
        noise;

    % Normalizacion
    signal = signal ./ max(abs(signal));

    % float32
    signal = single(signal);

    % GUARDAR SIGNAL.BIN
    fprintf('Guardando signal.bin...\n');

    signal_file = fullfile(dataset_path, 'signal.bin');

    fid = fopen(signal_file, 'w', 'ieee-le');

    if fid == -1
        error('No se pudo abrir signal.bin');
    end

    fwrite(fid, signal, 'float32');

    fclose(fid);

    % GENERAR FILTROS
    fprintf('Generando filtros FIR...\n');

    filters = zeros(NUM_FILTERS, FILTER_ORDER, 'single');

    cutoff_frequencies = linspace(300, 12000, NUM_FILTERS);

    for i = 1:NUM_FILTERS

        fc = cutoff_frequencies(i);

        % PASO BAJO
        if i <= 4

            coeffs = create_lowpass_fir( ...
                FILTER_ORDER, ...
                fc, ...
                SAMPLE_RATE ...
            );

        % PASO ALTO
        elseif i <= 8

            coeffs = create_highpass_fir( ...
                FILTER_ORDER, ...
                fc, ...
                SAMPLE_RATE ...
            );

        % PASA BANDA
        elseif i <= 12

            bw = 1000;

            f_low = max(100, fc - bw);
            f_high = min((SAMPLE_RATE/2)-100, fc + bw);

            coeffs = create_bandpass_fir( ...
                FILTER_ORDER, ...
                f_low, ...
                f_high, ...
                SAMPLE_RATE ...
            );

        % NOTCH
        else

            bw = 800;

            f_low = max(100, fc - bw);
            f_high = min((SAMPLE_RATE/2)-100, fc + bw);

            coeffs = create_notch_fir( ...
                FILTER_ORDER, ...
                f_low, ...
                f_high, ...
                SAMPLE_RATE ...
            );

        end

        filters(i, :) = single(coeffs);

    end

    % GUARDAR FILTERS.BIN
    fprintf('Guardando filters.bin...\n');

    filters_file = fullfile(dataset_path, 'filters.bin');

    fid = fopen(filters_file, 'w', 'ieee-le');

    if fid == -1
        error('No se pudo abrir filters.bin');
    end

    %
    % Layout:
    %
    % [Filtro 0 completo]
    % [Filtro 1 completo]
    % ...
    fwrite(fid, filters', 'float32');

    fclose(fid);

    % CONFIG
    fprintf('Guardando config.txt...\n');

    config_file = fullfile(dataset_path, 'config.txt');

    fid = fopen(config_file, 'w');

    fprintf(fid, 'DATASET=%s\n', cfg.name);
    fprintf(fid, 'SIGNAL_SIZE=%d\n', N);
    fprintf(fid, 'NUM_FILTERS=%d\n', NUM_FILTERS);
    fprintf(fid, 'FILTER_ORDER=%d\n', FILTER_ORDER);
    fprintf(fid, 'SAMPLE_RATE=%d\n', SAMPLE_RATE);
    fprintf(fid, 'DATA_TYPE=float32\n');
    fprintf(fid, 'ENDIAN=little-endian\n');

    fclose(fid);

    % INFO
    signal_size_mb = (N * 4) / (1024^2);

    filters_size_mb = ...
        (NUM_FILTERS * FILTER_ORDER * 4) / (1024^2);

    fprintf('\nDataset generado correctamente.\n');

    fprintf('Signal size:  %.2f MB\n', signal_size_mb);
    fprintf('Filters size: %.2f MB\n', filters_size_mb);

    fprintf('\nArchivos guardados en:\n');
    fprintf('%s\n', dataset_path);

end

% FINAL
fprintf('\n=========================================\n');
fprintf('Todos los datasets fueron generados.\n');
fprintf('=========================================\n');

% FUNCIONES AUXILIARES

% LOWPASS FIR
function h = create_lowpass_fir(N, fc, fs)

M = N - 1;

n = 0:M;

fc_norm = fc / fs;

h = 2 * fc_norm * sinc_custom(2 * fc_norm * (n - M/2));

w = custom_hamming(N);

h = h .* w;

h = h / sum(h);

end

% HIGHPASS FIR
function h = create_highpass_fir(N, fc, fs)

h_lp = create_lowpass_fir(N, fc, fs);

h = -h_lp;

center = floor(N/2) + 1;

h(center) = h(center) + 1;

end

% BANDPASS FIR
function h = create_bandpass_fir(N, f1, f2, fs)

h_lp1 = create_lowpass_fir(N, f1, fs);

h_lp2 = create_lowpass_fir(N, f2, fs);

h = h_lp2 - h_lp1;

end

% NOTCH FIR
function h = create_notch_fir(N, f1, f2, fs)

h_bp = create_bandpass_fir(N, f1, f2, fs);

h = -h_bp;

center = floor(N/2) + 1;

h(center) = h(center) + 1;

end

% HAMMING WINDOW
function w = custom_hamming(N)

n = 0:(N-1);

w = 0.54 - 0.46 * cos((2*pi*n)/(N-1));

end

% SINC CUSTOM
function y = sinc_custom(x)

y = ones(size(x));

idx = (x ~= 0);

y(idx) = sin(pi * x(idx)) ./ (pi * x(idx));

end