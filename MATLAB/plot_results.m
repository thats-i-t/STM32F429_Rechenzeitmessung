figure(4343)
clf(4343)
Nrows = 4;
Ncols = 2;

t_rcv = squeeze(t_rcv);
MrefLim = squeeze(MrefLim);
IdRef = squeeze(IdRef);
IqRef = squeeze(IqRef);
calcTime = squeeze(calcTime);
speedLoopback = squeeze(speedLoopback);
testVar = squeeze(testVar);
strategy = squeeze(strategy);

MAGIC_NUMBER = 123.4;
% valid = abs(testVar - MAGIC_NUMBER) < 1e-5 & abs(t_rcv) < 10;
valid = abs(testVar - IdRef(:,1) - IqRef(:,1)) < 1e-5 & abs(t_rcv) < 10 & abs(t_rcv) > 1;

subplot(Nrows,Ncols,1)
hold on
stairs(t_calc, Mref, 'k-', 'LineWidth', 1)
stairs(t_rcv(valid), MrefLim(valid,1), 'b.', 'LineWidth', 1)
% stairs(t_calc, MrefLim(:,2), 'r--', 'LineWidth', 0.5)
xlabel('Sim-Time (s)')
ylabel('M_{ref,lim} (Nm)')
ylim([-3 3])

subplot(Nrows,Ncols,2)
hold on
stairs(t_calc, nMech, 'k-', 'LineWidth', 1)
% stairs(t_rcv(valid), speedLoopback(valid), 'b.', 'LineWidth', 0.5)
xlabel('Sim-Time (s)')
ylabel('n_{mech} (min^{-1})')

subplot(Nrows,Ncols,3)
hold on
stairs(t_rcv(valid), IdRef(valid,1), 'b.', 'LineWidth', 1)
stairs(t_calc, IdRef(:,2), 'g-', 'LineWidth', 0.5)
xlabel('Sim-Time (s)')
ylabel('i_{d,ref} (A)')

subplot(Nrows,Ncols,4)
hold on
stairs(t_rcv(valid), IqRef(valid,1), 'b.', 'LineWidth', 1)
stairs(t_calc, IqRef(:,2), 'g-','LineWidth', 0.5)
xlabel('Sim-Time (s)')
ylabel('i_{q,ref} (A)')

subplot(Nrows,Ncols,5)
hold on
stairs(t_rcv(valid), strategy(valid), 'b.', 'LineWidth', 1)
xlabel('Sim-Time (s)')
ylabel('strategy')

subplot(Nrows,Ncols,6)
hold on
stairs(t_rcv(valid), calcTime(valid), 'b.', 'LineWidth', 1)
xlabel('Sim-Time (s)')
ylabel('Calc. time (us)')

Rs = 15e-3;
Ld = 80e-6;
Lq = 90e-6;
PsiP = 5e-3;
Np = 7;
Uzk = 50;
Imax = 70;

subplot(Nrows,Ncols,7)
hold on
stairs([t(1) t(end)], [1 1]*Imax, 'r-', 'LineWidth', 1)
stairs(t_rcv(valid), abs(IdRef(valid,1) + 1i*IqRef(valid,1)), 'b.', 'LineWidth', 1)
% stairs(t_calc, IdRef(:,2), 'r--', 'LineWidth', 0.5)
xlabel('Sim-Time (s)')
ylabel('i_{abs,ref} (A)')

w = speedLoopback(valid);
Ud = Rs*IdRef(valid,1)-w*Lq.*IqRef(valid,1);
Uq = Rs*IqRef(valid,1)+w*PsiP+w*Ld.*IdRef(valid,1);
t = t_rcv(valid);
subplot(Nrows,Ncols,8)
hold on
stairs([t(1) t(end)], [1 1]*Uzk/sqrt(3), 'r-', 'LineWidth', 1)
stairs(t_rcv(valid), abs(Ud + 1i * Uq), 'b.', 'LineWidth', 1)
xlabel('Sim-Time (s)')
ylabel('u_{abs,ref} (A)')
