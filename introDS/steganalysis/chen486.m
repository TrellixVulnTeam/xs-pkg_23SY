function f = chen486(IMAGE)
% -------------------------------------------------------------------------
% Copyright (c) 2011 DDE Lab, Binghamton University, NY.
% All Rights Reserved.
% -------------------------------------------------------------------------
% Permission to use, copy, modify, and distribute this software for
% educational, research and non-profit purposes, without fee, and without a
% written agreement is hereby granted, provided that this copyright notice
% appears in all copies. The program is supplied "as is," without any
% accompanying services from DDE Lab. DDE Lab does not warrant the
% operation of the program will be uninterrupted or error-free. The
% end-user understands that the program was developed for research purposes
% and is advised not to rely exclusively on the program for any reason. In
% no event shall Binghamton University or DDE Lab be liable to any party
% for direct, indirect, special, incidental, or consequential damages,
% including lost profits, arising out of the use of this software. DDE Lab
% disclaims any warranties, and has no obligations to provide maintenance,
% support, updates, enhancements or modifications.
% -------------------------------------------------------------------------
% Contact: jan@kodovsky.com | fridrich@binghamton.edu | November 2011
%          http://dde.binghamton.edu/download/feature_extractors
% -------------------------------------------------------------------------
% Extracts 486 inter- and intra-block DCT domain features proposed in [1].
% Inter-block part (324) is based on [2].
% -------------------------------------------------------------------------
% Input:  IMAGE ... path to the JPEG image
% Output: f ....... extracted feature vector
% -------------------------------------------------------------------------
% IMPORTANT: For accessing DCT coefficients of JPEG images, we use Phil
% Sallee's Matlab JPEG toolbox (function jpeg_read) available at
% http://philsallee.com/jpegtbx/
% -------------------------------------------------------------------------
% [1] C. Chen and Y. Q. Shi. "JPEG image steganalysis utilizing both
%     intrablock and interblock correlations," IEEE ISCAS, International
%     Symposium on Circuits and Systems, pages 3029�3032, May 2008.
% [2] Y. Q. Shi, C. Chen, and W. Chen. "A Markov process based approach to
%     effective attacking JPEG steganography," In J. L. Camenisch, C. S.
%     Collberg, N. F. Johnson, and P. Sallee, editors, Information Hiding,
%     8th International Workshop, volume 4437 of Lecture Notes in Computer
%     Science, pages 249�264, Alexandria, VA, July 10�12, 2006.
%     Springer-Verlag, New York.
% -------------------------------------------------------------------------

X = DCTPlane(IMAGE);
F1 = extractShi324(X); % intra-block part [2]
F2 = extract_interblock(X); % inter-block part [1]
f = [F1;F2];

function F = extract_interblock(A)
T = 4; % threshold
F = zeros(2*(2*T+1)^2,1); % inter-block part
for MODE = 2:64
    % inter-block part (for single mode)
    % obtain appropriate mode 2-d array (see [1])
    AA = PlaneToVecMode(A,MODE);
    % horizontal (2T+1)^2 features
    Ad = conv2(AA,[-1 1],'valid');   % <=> A(:,1:end-1) - A(:,2:end);
    Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
    Fah = getTPM(Ad(:,1:end-1),Ad(:,2:end),T);         % add to the feature vector

    % vertical (2T+1)^2 features
    Ad = conv2(AA,[-1;1],'valid');    % <=> A(1:end-1,:) - A(2:end,:);
    Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
    Fav = getTPM(Ad(1:end-1,:),Ad(2:end,:),T);       % add to the feature vector

    F = F + [Fah;Fav];                   % add to the final vector
end
F = F/63; % take average over all modes
function F = extractShi324(A)
% extract [2] features from DCTPlane A
A = abs(A);
T = 4; F = [];

%% horizontal (2T+1)^2 features
Ad = conv2(A,[-1 1],'valid');    % <=> A(:,1:end-1) - A(:,2:end);
Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
A1 = Ad(:,1:end-1);
A2 = Ad(:,2:end);
F = [F;getTPM(A1,A2,T)];  % Markov chain

%% vertical (2T+1)^2 features
Ad = conv2(A,[-1;1],'valid');    % <=> A(1:end-1,:) - A(2:end,:);
Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
A1 = Ad(1:end-1,:);
A2 = Ad(2:end,:);
F = [F;getTPM(A1,A2,T)];  % Markov chain

%% diagonal (2T+1)^2 features
Ad = conv2(A,[-1 0;0 1],'valid');% <=> A(1:end-1,1:end-1) - A(2:end,2:end);
Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
A1 = Ad(1:end-1,1:end-1);
A2 = Ad(2:end,2:end);
F = [F;getTPM(A1,A2,T)]; % Markov chain

%% minor diagonal (2T+1)^2 features
Ad = conv2(A,[0 1;-1 0],'valid');% <=> A(2:end,1:end-1) - A(1:end-1,2:end);
Ad = max(min(Ad,T),-T);          % truncate to [-T,T]
A1 = Ad(2:end,1:end-1);
A2 = Ad(1:end-1,2:end);
F = [F;getTPM(A1,A2,T)]; % Markov chain
function F = getTPM(A1,A2,T)
% get transition probability matrix A1 --> A2, range -T..T
F = zeros(2*T+1);
dn = max(hist(A1(:),-T:T),1); % normalization factors
for i=-T:T
    FF = A2(A1==i); % filtered version
    for j=-T:T
        F(i+T+1,j+T+1) = nnz(FF==j)/dn(i+T+1);
    end
end
F = F(:);
function Mat=PlaneToVecMode(plane,MODE)
mask = reshape(1:64,8,8);
[i,j] = find(mask==MODE);
Mat = plane(i:8:end,j:8:end);
function Plane=DCTPlane(path)
% loads DCT Plane of the given JPEG image
jobj=jpeg_read(path); % Phil Sallee's MATLAB jpeg toolbox needed
Plane=jobj.coef_arrays{1};
