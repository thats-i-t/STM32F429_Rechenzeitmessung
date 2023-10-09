figure(4343)
clf(4343)
Nrows = 3;
Ncols = 2;

t_rcv = squeeze(t_rcv);
MrefLim = squeeze(MrefLim);
IdRef = squeeze(IdRef);
IqRef = squeeze(IqRef);
calcTime = squeeze(calcTime);

MAGIC_NUMBER = 123.4;
valid = abs(testVar - MAGIC_NUMBER) < 1e-5;

subplot(Nrows,Ncols,1)
hold on
stairs(t_calc, Mref, 'k-', 'LineWidth', 1)
stairs(t_rcv(valid), MrefLim(valid,1), 'b.-', 'LineWidth', 1)
stairs(t_calc, MrefLim(:,2), 'r--', 'LineWidth', 0.5)
xlabel('Samples')
ylabel('M_{ref,lim} (Nm)')

subplot(Nrows,Ncols,2)
hold on
stairs(t_calc, nMech, 'k-', 'LineWidth', 1)
xlabel('Samples')
ylabel('n_{mech} (min^{-1})')

subplot(Nrows,Ncols,3)
hold on
stairs(t_rcv(valid), IdRef(valid,1), 'b-', 'LineWidth', 1)
stairs(t_calc, IdRef(:,2), 'r--', 'LineWidth', 0.5)
xlabel('Samples')
ylabel('i_{d,ref} (A)')

subplot(Nrows,Ncols,4)
hold on
stairs(t_rcv(valid), IqRef(valid,1), 'b-', 'LineWidth', 1)
stairs(t_calc, IqRef(:,2), 'r--','LineWidth', 0.5)
xlabel('Samples')
ylabel('i_{q,ref} (A)')

subplot(Nrows,Ncols,5)
hold on
stairs(t_rcv(valid), calcTime(valid), 'b-', 'LineWidth', 1)
xlabel('Samples')
ylabel('Calc. time (us)')
