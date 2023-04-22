
class PeakFilter:
    """ Pre-definied peak filter class for BPM """
    __A_COEFS = [-1.998072905031434, 0.998112346058487]
    __B_COEFS = [9.438269707564562e-04, 0, -9.438269707564562e-04]

    def __init__(self):
        self.__len_x = len(self.__A_COEFS)
        self.__len_y = len(self.__B_COEFS)
        self.__x = [0.0] * self.__len_x
        self.__y = [0.0] * self.__len_y
        self.__res = 0.0

    
    def __str__(self) -> str:
        return f"Peakfilter:\n\ta: {self.__A_COEFS}\n\tb: {self.__B_COEFS}"


    def filter_sample(self, sample) -> float:
        self.__x = [sample] + [self.__x[ii] for ii in range(self.__len_x - 1)]
        self.__y = [self.__res] + [self.__y[ii] for ii in range(self.__len_y - 1)]
        tmp_b = sum([self.__B_COEFS[ii] * self.__x[ii] for ii in range(self.__len_x)])
        tmp_a = sum([self.__A_COEFS[ii] * self.__y[ii] for ii in range(self.__len_y)])
        self.__res = tmp_b - tmp_a
        return self.__res


    def filter_array(self, arr) -> list:
        res = []
        for sample in arr:
            res.append(self.filter_sample(sample))
        return res