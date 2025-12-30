import torch
import torch.nn.functional as F


class MLP(torch.nn.Module):
    def __init__(self, in_features, hid_features, out_features):
        super(MLP, self).__init__()

        self.fc1 = torch.nn.Linear(in_features=in_features, out_features=hid_features, bias=True)
        self.fc2 = torch.nn.Linear(in_features=hid_features, out_features=hid_features, bias=True)
        self.fc3 = torch.nn.Linear(in_features=hid_features, out_features=out_features, bias=True)

        torch.nn.init.zeros_(self.fc1.bias)
        torch.nn.init.zeros_(self.fc2.bias)
        torch.nn.init.zeros_(self.fc3.bias)

        self.fc1.weight = torch.nn.Parameter(torch.zeros((in_features, hid_features), dtype=torch.float32).uniform_(-0.05, 0.05).T,
                                             requires_grad=True)
        self.fc2.weight = torch.nn.Parameter(torch.zeros((hid_features, hid_features), dtype=torch.float32).uniform_(-0.05, 0.05).T,
                                             requires_grad=True)
        self.fc3.weight = torch.nn.Parameter(torch.zeros((hid_features, out_features), dtype=torch.float32).uniform_(-0.05, 0.05).T,
                                             requires_grad=True)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = F.relu(torch.exp(self.fc3(x)) - 1.0)  # additional relu is max() op as in code in nn.h
        return x