(Screen Name="ProfileForm" Width=390 Height=844 ResizeMode="CanResize"
  ClearColor="$background" Controller="MvvmForms/Scripts/Form"
  (StackView Name="DraftForm" Padding="24" Spacing=12 Overflow="hidden scroll"
    ForegroundColor="$foreground" DataContextSource="MvvmForms/Models/Profile:."
    (TextBlock FontSize=30 FontWeight="Bold" Text="Your profile")
    (TextBlock FontSize=15 ForegroundColor="$muted" Text="Changes stay in this draft until you save.")
    (TextBlock MarginTop=16 Text="Display name")
    (Input Name="DisplayName" Height=48 Text=(bind "DataContext/DisplayName" Mode=TwoWay))
    (TextBlock Text="Email address")
    (Input Name="Email" Type="Email" Height=48 Text=(bind "DataContext/Email" Mode=TwoWay))
    (Grid Columns="32px 1fr" Height=48 Spacing=12
      (Input Name="OptIn" Type="Checkbox" Checked=(bind "DataContext/OptIn" Mode=TwoWay))
      (TextBlock Text="Send me updates" TextVerticalAlignment="Center"))
    (TextBlock MarginTop=12 ForegroundColor="$muted" Text="Preview")
    (TextBlock Name="Preview" FontSize=20 Text=(bind "DataContext/DisplayName"))
    (Grid Columns="1fr 1fr" Height=48 Spacing=12 MarginTop=12
      (Button Name="Save" Text="Save" BackgroundColor="$accent" ForegroundColor="#ffffff" Click="Save")
      (Button Name="Cancel" Text="Discard changes" Click="Cancel"))
    (TextBlock Name="Status" MarginTop=12 Text=(bind "DataContext/Status"))))
