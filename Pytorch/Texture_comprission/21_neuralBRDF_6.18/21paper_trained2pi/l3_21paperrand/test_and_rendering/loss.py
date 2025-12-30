import torch

class Loss:
    def __init__(self, target, output):
        self.target = target
        self.output = output

    def train_MSE_loss(self):
        return torch.mean((self.target - self.output) ** 2)

    def train_RMSE_loss(self):
        return torch.sqrt(torch.mean((self.target - self.output) ** 2))

    def train_MAE_loss(self):
        return torch.mean(torch.abs(self.target - self.output))

    def train_logMAE_loss(self):
        return torch.mean(torch.abs(torch.log(1+self.target)- torch.log(1+self.output)))

    def train_logMSE_loss(self):
        return torch.mean((torch.log(1+self.target)- torch.log(1+self.output))**2)

    def train_logRMSE_loss(self):
        return torch.sqrt(torch.mean((torch.log(1+self.target)- torch.log(1+self.output))**2))

    def train_logawareMAE_loss(self, A, B):
        log_target = (torch.log(self.target+0.01)-torch.log(A)) / (torch.log(B) - torch.log(A))
        log_output = (torch.log(self.output+0.01)-torch.log(A)) / (torch.log(B) - torch.log(A))

        return torch.mean(torch.abs(log_target - log_output))

    def test_loss(self):
        return torch.mean(torch.abs(self.target - self.output))

    def test_RMSE_loss(self):
        return torch.sqrt(torch.mean((self.target - self.output)**2))

    def test_log_loss(self):
        return torch.mean(torch.abs(torch.log(1+self.target)- torch.log(1+self.output)))

    def train_loss_1(self):
        target = (self.target - 0.5) / 0.5
        output = (self.output - 0.5) / 0.5
        return torch.mean((target-output)**2)

    def mean_absolute_logarithmic_error(self, y_true, y_pred):
        return torch.mean(torch.abs(torch.log(1 + y_true) - torch.log(1 + y_pred)))