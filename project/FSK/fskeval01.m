%
%
%
%

if ~exist('tracebuffer')
    load tracebuffer.dat
    tb = reshape(tracebuffer, 4,length(tracebuffer)/4 )'; size(tb)
end


%---- do receiver ---------------------------
Fbit = 1.44;			% bit rate freq in kHz
Fsm = [74 63.3];		% space and mark FSK carrier frequencies in kHz
Fs = 60.48;

fmode = 1;
Ntx = length(tb);
Ndelay = 2;
Nfilt = 11;

switch fmode
case 1
	Bf = [ones(1,Nfilt)/Nfilt];		% define filter tap weights that average for 1 period
	Af = 1;
case 2
	%---- 5th order IIR lowpass ----------
    Bf = [  0        0.0045    0.0054   -0.0145    0.0092    0.0023 ];
    Af = [ 1.0000   -3.6449    5.4525   -4.1606    1.6139   -0.2540 ];
	Nfilt = length(Bf);
end


%b = 0.07;
%b = 0;

rx = zeros(Ntx,1);			% init receive vector
adcSignal = tb(:,1);

for n = 1 : Ntx-1-Ndelay
	rx(n) = adcSignal(n+Ndelay) .* adcSignal(n);	% multiply by delayed version of signal	
	rx(n) = rx(n) + b*adcSignal(n).*adcSignal(n);
end

rxf = filter(Bf, Af, rx)/1e6;		% low pass filter

t = [0 : length(rxf)-1]/Fs*Fbit;
figure(2);						% plot result
subplot(1,1,1);
plot(t, rxf,'.-')
xlabel('bit times')
ylabel('demod data')
title(sprintf('RX demod signal for Tdelay = %d samples and Nfilt = %d samples',...
	Ndelay, Nfilt ))
grid on
%set(gca, 'YLIM', [-0.5 0.5])


    
