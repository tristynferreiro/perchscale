% Open the text file for reading
fileID = fopen('/Users/mishaynaidoo/Desktop/UCT/EEE4113F/perch_scale/Kalman_Filter/test_scale.txt', 'r');

% Initialize an empty cell array to store the arrays
allData = {};

% Read the entire file content
content = fileread('/Users/mishaynaidoo/Desktop/UCT/EEE4113F/perch_scale/Kalman_Filter/test_scale.txt');

% Split the content into sections between "done" keywords
sections = strsplit(content, 'done');

% Process each section
for i = 1:numel(sections)
    % Extract the numbers from the section
    numbers = str2num(sections{i});
    
    % Store the numbers in the cell array
    allData{i} = numbers;
end

% Close the file
fclose(fileID);

% Display the arrays containing the data between "done" keywords
% for i = 1:numel(allData)
%     disp(allData{i});
% end

plot(allData{5})
hold on
Est = 350;

Eest = 200;

Emeas = 50;

Estimates = [];

for i = 1:length(allData{5})
    meas = allData{5}(i);
    if meas>250 & meas < 350
        k = Eest/(Eest+Emeas);
        Est = Est + k*(meas-Est);
        Eest = (1-k)*(Eest);
        Estimates(i) = Est;
    end

end

plot(Estimates)
hold off


