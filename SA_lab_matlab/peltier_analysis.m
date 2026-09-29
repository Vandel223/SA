%% =========================================================================
%  Peltier Module Thermal Analysis — MATLAB Script
%  Sensors and Actuators Laboratory, IST
%  =========================================================================
%  Files (all in the same folder as this script):
%    subida_ensaio.csv  — Rise transient (2 cols: T1, T2), no time stamp
%                         Sampling period dt = 0.5 s
%    descida_ensaio.csv — Fall transient (3 cols: t[s], T1, T2)
%    4V_ensaio.csv      — Steady-state @ 4 V   (3 cols: t[s], T1, T2)
%    4V5_ensaio.csv     — Steady-state @ 4.5 V (3 cols: t[s], T1, T2)
%  =========================================================================

clearvars; close all; clc;

%% =========================================================================
%  LOCAL HELPER FUNCTIONS
%% =========================================================================

function [T1, T2] = load2col(fname)
    T1 = [];  T2 = [];
    fid = fopen(fullfile('data', fname), 'r');
    while ~feof(fid)
        line = strtrim(fgetl(fid));
        parts = strsplit(line, ',');
        if numel(parts) < 2,  continue;  end
        vals = str2double(parts(1:2));
        if any(~isfinite(vals)) || any(abs(vals) > 100), continue; end
        T1(end+1,1) = vals(1);  %#ok<AGROW>
        T2(end+1,1) = vals(2);  %#ok<AGROW>
    end
    fclose(fid);
end

function [t, T1, T2] = load3col(fname)
    t = [];  T1 = [];  T2 = [];
    fid = fopen(fullfile('data', fname), 'r');
    while ~feof(fid)
        line = strtrim(fgetl(fid));
        parts = strsplit(line, ',');
        if numel(parts) < 3, continue; end
        vals = str2double(parts(1:3));
        if any(~isfinite(vals)) || vals(2) <= 0 || vals(3) <= 0, continue; end
        t(end+1,1)  = vals(1);  %#ok<AGROW>
        T1(end+1,1) = vals(2);  %#ok<AGROW>
        T2(end+1,1) = vals(3);  %#ok<AGROW>
    end
    fclose(fid);
    t = t - t(1);
end

function [t_out, T1_out, T2_out] = crop3(t, T1, T2, i_start, i_end)
    i_end  = min(i_end, numel(t));
    idx    = i_start:i_end;
    t_out  = t(idx) - t(i_start);
    T1_out = T1(idx);
    T2_out = T2(idx);
end

%% =========================================================================
%  1.  LOAD ALL DATA
%% =========================================================================

[T2_rise_all, T1_rise_all] = load2col('subida_ensaio.csv');
dt_s       = 0.5;
t_rise_all = (0 : numel(T1_rise_all)-1)' * dt_s;

[t_fall, T1_fall, T2_fall] = load3col('descida_ensaio.csv');
[t_4v,   T1_4v,   T2_4v  ] = load3col('4V_ensaio.csv');
[t_4v5,  T1_4v5,  T2_4v5 ] = load3col('4V5_ensaio.csv');

%% =========================================================================
%  2.  CROP PARAMETERS  (sample indices, 1-based, inclusive)
%  Adjust these to trim each dataset to its clean region of interest.
%% =========================================================================

% Rise (subida) — 2-col file, dt = 0.5 s
RISE_START = 300;
RISE_END   = 390;

% Fall (descida)
FALL_START = 11;
FALL_END   = inf;

% Steady-state 4.0 V
SS4V_START = 47;
SS4V_END   = 217;

% Steady-state 4.5 V
SS4V5_START = 11;
SS4V5_END   = 180;

%% =========================================================================
%  2b.  CURSOR TIMESTAMPS  (seconds, after crop/re-zero)
%  -------------------------------------------------------------------------
%  T_RISE_CURSOR : time of cold-side minimum during rise transient — marks
%                  the Peltier cooling onset before Joule heating takes over.
%                  Set to [] for auto-detection.
%
%  T_FALL_CURSOR : time of voltage step 4.5V -> 4.0V in the fall recording.
%                  Set to [] to use t = 0 (crop start).
%% =========================================================================

T_RISE_CURSOR = [15];   % [] = auto (min of cold side)
T_FALL_CURSOR = [66];   % [] = auto (t = 0)

%% =========================================================================
%  3.  APPLY CROPS
%% =========================================================================

i_end_rise = min(RISE_END, numel(T1_rise_all));
idx_rise   = RISE_START : i_end_rise;
T1_rise    = T1_rise_all(idx_rise);
T2_rise    = T2_rise_all(idx_rise);
dT_rise    = abs(T1_rise - T2_rise);
t_rise     = (0 : numel(T1_rise)-1)' * dt_s;

[t_fall, T1_fall, T2_fall] = crop3(t_fall, T1_fall, T2_fall, FALL_START, FALL_END);
[t_4v,   T1_4v,   T2_4v  ] = crop3(t_4v,   T1_4v,   T2_4v,   SS4V_START,  SS4V_END);
[t_4v5,  T1_4v5,  T2_4v5 ] = crop3(t_4v5,  T1_4v5,  T2_4v5,  SS4V5_START, SS4V5_END);

dT_fall = abs(T1_fall - T2_fall);
dT_4v   = abs(T1_4v   - T2_4v);
dT_4v5  = abs(T1_4v5  - T2_4v5);

%% =========================================================================
%  4.  CURSOR AUTO-DETECTION
%% =========================================================================

if isempty(T_RISE_CURSOR)
    T_cold_rise = min(T1_rise, T2_rise);
    [~, idx_rise_cur] = min(T_cold_rise);
    T_RISE_CURSOR = t_rise(idx_rise_cur);
else
    [~, idx_rise_cur] = min(abs(t_rise - T_RISE_CURSOR));
end
rise_cur_T1 = T1_rise(idx_rise_cur);
rise_cur_T2 = T2_rise(idx_rise_cur);

if isempty(T_FALL_CURSOR)
    T_FALL_CURSOR = t_fall(1);
end
[~, idx_fall_cur] = min(abs(t_fall - T_FALL_CURSOR));
fall_cur_T1 = T1_fall(idx_fall_cur);
fall_cur_T2 = T2_fall(idx_fall_cur);

%% =========================================================================
%  5.  STEADY-STATE SUMMARY  (median of last 40 %)
%% =========================================================================

ss_frac   = 0.60;
idx_ss4v  = round(ss_frac * numel(dT_4v));
idx_ss4v5 = round(ss_frac * numel(dT_4v5));

dT_4v_ss  = median(dT_4v(idx_ss4v:end));
dT_4v5_ss = median(dT_4v5(idx_ss4v5:end));

T1_4v_ss_val  = median(T1_4v(idx_ss4v:end));
T2_4v_ss_val  = median(T2_4v(idx_ss4v:end));
T1_4v5_ss_val = median(T1_4v5(idx_ss4v5:end));
T2_4v5_ss_val = median(T2_4v5(idx_ss4v5:end));

fprintf('=== Steady-state summary ===\n');
fprintf('  dT @ 4.0 V : %.2f C\n',  dT_4v_ss);
fprintf('  dT @ 4.5 V : %.2f C\n',  dT_4v5_ss);
fprintf('  Difference  : %.2f C  (%.1f%%)\n\n', ...
        dT_4v5_ss - dT_4v_ss, 100*(dT_4v5_ss - dT_4v_ss)/dT_4v_ss);
fprintf('  T_hot  @ 4.0 V SS: %.2f C | T_cold: %.2f C\n', T1_4v_ss_val, T2_4v_ss_val);
fprintf('  T_hot  @ 4.5 V SS: %.2f C | T_cold: %.2f C\n', T1_4v5_ss_val, T2_4v5_ss_val);

%% =========================================================================
%  6.  GLOBAL RENDERING DEFAULTS
%  -------------------------------------------------------------------------
%  Set LaTeX as the interpreter for ALL text objects globally.
%  Font sizes are controlled via the 'FontSize' Name-Value pair — MATLAB's
%  LaTeX subset does not support \Large, \huge etc., but does support
%  \textbf{} for bold and standard math mode ($...$) for symbols.
%% =========================================================================

FONT_TITLE  = 13;   % title font size
FONT_LABEL  = 11;   % axis label font size
FONT_TICK   = 10;   % tick label / general axes font size
FONT_ANNOT  = 9;    % small annotation text

set(groot, ...
    'DefaultAxesFontSize',         FONT_TICK, ...
    'DefaultLineLineWidth',        1.6, ...
    'DefaultAxesGridAlpha',        0.25, ...
    'DefaultAxesGridLineStyle',    '--', ...
    'DefaultTextInterpreter',      'latex', ...   % text() calls
    'DefaultAxesTickLabelInterpreter', 'latex', ...
    'DefaultLegendInterpreter',    'latex');

% Helper: wrap a plain string for use with the latex interpreter
% (no special characters needed — just passes through cleanly)

c_hot  = [0.85  0.18  0.10];
c_cold = [0.12  0.47  0.71];
c_dT   = [0.49  0.18  0.56];
c_4v   = [0.17  0.63  0.17];
c_45v  = [0.85  0.33  0.10];
c_cur  = [0.00  0.00  0.00];

cur_marker = 'v';
cur_ms     = 9;

%% =========================================================================
%  FIGURE 1 — Rise Transient
%% =========================================================================

f1 = figure('Name','Rise Transient','Position',[40 40 1000 420]);

% ---- (a) Sensor temperatures ----
ax1 = subplot(2,1,1);
hold on; grid on; box on;
plot(t_rise, T1_rise, 'Color', c_hot,  'DisplayName', '$T_1$ (hot side)');
plot(t_rise, T2_rise, 'Color', c_cold, 'DisplayName', '$T_2$ (cold side)');

xline(T_RISE_CURSOR, '--', 'Color', c_cur, 'LineWidth', 1.2, ...
      'HandleVisibility','off');
plot(T_RISE_CURSOR, rise_cur_T1, cur_marker, ...
     'Color', c_hot,  'MarkerFaceColor', c_hot,  'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
plot(T_RISE_CURSOR, rise_cur_T2, cur_marker, ...
     'Color', c_cold, 'MarkerFaceColor', c_cold, 'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
text(T_RISE_CURSOR + 0.5, rise_cur_T1 - 1.5, ...
     sprintf('$T_1 = %.2f\\,^\\circ$C', rise_cur_T1), ...
     'FontSize', FONT_ANNOT, 'Color', c_hot,  'VerticalAlignment','middle');
text(T_RISE_CURSOR + 0.5, rise_cur_T2 + 1.5, ...
     sprintf('$T_2 = %.2f\\,^\\circ$C', rise_cur_T2), ...
     'FontSize', FONT_ANNOT, 'Color', c_cold, 'VerticalAlignment','middle');
ylim([22 40]);
text(T_RISE_CURSOR, ax1.YLim(1), ...
     sprintf('$t = %.1f$ s', T_RISE_CURSOR), ...
     'FontSize', FONT_ANNOT, 'Color', c_cur, ...
     'VerticalAlignment','bottom', 'HorizontalAlignment','left');
xlabel('Time (s)', 'FontSize', FONT_LABEL);
ylabel('Temperature ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{(a) Rise Transient -- Sensor Temperatures}', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','northwest', 'FontSize', FONT_ANNOT);

% ---- (b) Delta T ----
ax2 = subplot(2,1,2);
hold on; grid on; box on;
plot(t_rise, dT_rise, 'Color', c_dT, 'DisplayName', '$|\Delta T|$');

xline(T_RISE_CURSOR, '--', 'Color', c_cur, 'LineWidth', 1.2, ...
      'HandleVisibility','off');
dT_at_cur = dT_rise(idx_rise_cur);
plot(T_RISE_CURSOR, dT_at_cur, cur_marker, ...
     'Color', c_dT, 'MarkerFaceColor', c_dT, 'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
text(T_RISE_CURSOR + 0.5, dT_at_cur - 0.5, ...
     sprintf('$|\\Delta T| = %.2f\\,^\\circ$C', dT_at_cur), ...
     'FontSize', FONT_ANNOT, 'Color', c_dT, 'VerticalAlignment','middle');

xlabel('Time (s)', 'FontSize', FONT_LABEL);
ylabel('$|\Delta T|$ ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{(b) Rise Transient -- $|\Delta T|$}', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','southeast', 'FontSize', FONT_ANNOT);

exportgraphics(f1, 'fig1_rise_transient.pdf', 'ContentType','vector');
fprintf('Saved fig1_rise_transient.pdf\n');

%% =========================================================================
%  FIGURE 2 — Fall Transient
%% =========================================================================

f2 = figure('Name','Fall Transient','Position',[60 60 900 600]);

ax3 = subplot(2,1,1);
hold on; grid on; box on;
plot(t_fall, T1_fall, 'Color', c_hot,  'DisplayName', '$T_1$ (hot side)');
plot(t_fall, T2_fall, 'Color', c_cold, 'DisplayName', '$T_2$ (cold side)');

xline(T_FALL_CURSOR, '--', 'Color', c_cur, 'LineWidth', 1.4, ...
      'HandleVisibility','off');
plot(T_FALL_CURSOR, fall_cur_T1, cur_marker, ...
     'Color', c_hot,  'MarkerFaceColor', c_hot,  'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
plot(T_FALL_CURSOR, fall_cur_T2, cur_marker, ...
     'Color', c_cold, 'MarkerFaceColor', c_cold, 'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
text(T_FALL_CURSOR + 0.5, fall_cur_T1 - 1.5, ...
     sprintf('$T_1 = %.2f\\,^\\circ$C', fall_cur_T1), ...
     'FontSize', FONT_ANNOT, 'Color', c_hot,  'VerticalAlignment','middle');
text(T_FALL_CURSOR + 0.5, fall_cur_T2 + 1.5, ...
     sprintf('$T_2 = %.2f\\,^\\circ$C', fall_cur_T2), ...
     'FontSize', FONT_ANNOT, 'Color', c_cold, 'VerticalAlignment','middle');
text(T_FALL_CURSOR, ax3.YLim(1), ...
     '$4.5\,\mathrm{V} \to 4.0\,\mathrm{V}$', ...
     'FontSize', FONT_ANNOT, 'Color', c_cur, ...
     'VerticalAlignment','bottom', 'HorizontalAlignment','left');

xlabel('Time (s)', 'FontSize', FONT_LABEL);
ylabel('Temperature ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{Fall Transient -- Sensor Temperatures\ (voltage step at $t=66$}s\textbf{)}', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','east', 'FontSize', FONT_ANNOT);

ax4 = subplot(2,1,2);
hold on; grid on; box on;
plot(t_fall, dT_fall, 'Color', c_dT, 'DisplayName', '$|\Delta T|$');

xline(T_FALL_CURSOR, '--', 'Color', c_cur, 'LineWidth', 1.4, ...
      'HandleVisibility','off');
dT_fall_at_cur = dT_fall(idx_fall_cur);
plot(T_FALL_CURSOR, dT_fall_at_cur, cur_marker, ...
     'Color', c_dT, 'MarkerFaceColor', c_dT, 'MarkerSize', cur_ms, ...
     'HandleVisibility','off');
text(T_FALL_CURSOR + 0.5, dT_fall_at_cur - 1, ...
     sprintf('$|\\Delta T| = %.2f\\,^\\circ$C', dT_fall_at_cur), ...
     'FontSize', FONT_ANNOT, 'Color', c_dT, 'VerticalAlignment','middle');

t_text = t_fall(round(end*4/5));
text(t_text, median(dT_fall)*0.75, ...
     {'Joule heating ($I^2R$) raises both sides;', ...
      '$|\Delta T|$ rises rather than falls', ...
      '--- cooling effect is masked.'}, ...
     'FontSize', FONT_ANNOT, 'HorizontalAlignment','center', ...
     'BackgroundColor',[1 1 0.85],'EdgeColor',[0.6 0.6 0.6]);

xlabel('Time (s)', 'FontSize', FONT_LABEL);
ylabel('$|\Delta T|$ ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{Fall Transient -- $|\Delta T|$}:\ \textbf{Joule heating dominates}', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','southeast', 'FontSize', FONT_ANNOT);

exportgraphics(f2, 'fig2_fall_transient.pdf', 'ContentType','vector');
fprintf('Saved fig2_fall_transient.pdf\n');

%% =========================================================================
%  FIGURE 3 — Steady-State Voltage Comparison (4 V vs 4.5 V)
%% =========================================================================

f3 = figure('Name','Steady-State Comparison','Position',[80 80 950 640]);

ax5 = subplot(2,1,1);
hold on; grid on; box on;
plot(t_4v,  T1_4v,  '-',  'Color', c_4v,  'DisplayName', '$T_1$ @ 4.0\,V');
plot(t_4v,  T2_4v,  '--', 'Color', c_4v,  'DisplayName', '$T_2$ @ 4.0\,V');
plot(t_4v5, T1_4v5, '-',  'Color', c_45v, 'DisplayName', '$T_1$ @ 4.5\,V');
plot(t_4v5, T2_4v5, '--', 'Color', c_45v, 'DisplayName', '$T_2$ @ 4.5\,V');
xlabel('Elapsed Time (s)', 'FontSize', FONT_LABEL);
ylabel('Temperature ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{Steady-State Temperature Profiles:}\ 4.0\,V vs.\ 4.5\,V \textbf{Supply}', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','east', 'NumColumns', 2, 'FontSize', FONT_ANNOT);

ax6 = subplot(2,1,2);
hold on; grid on; box on;
plot(t_4v,  dT_4v,  '-', 'Color', c_4v,  'DisplayName', '$|\Delta T|$ @ 4.0\,V');
plot(t_4v5, dT_4v5, '-', 'Color', c_45v, 'DisplayName', '$|\Delta T|$ @ 4.5\,V');
% Steady-state reference lines omitted: |Delta T| has not stabilised
% due to insufficient heat-sink thermal mass (see thermal drift note above).
xlabel('Elapsed Time (s)', 'FontSize', FONT_LABEL);
ylabel('$|\Delta T|$ ($^\circ$C)', 'FontSize', FONT_LABEL);
title('\textbf{$|\Delta T|$ Comparison:}\ 4.0\,V vs.\ 4.5\,V', ...
      'FontSize', FONT_TITLE, 'Interpreter', 'latex');
legend('Location','east', 'FontSize', FONT_ANNOT);

exportgraphics(f3, 'fig3_steady_state.pdf', 'ContentType','vector');
fprintf('Saved fig3_steady_state.pdf\n');

fprintf('\n== All figures exported. ==\n');